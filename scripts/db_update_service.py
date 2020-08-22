#!/usr/bin/python3
import zmq
import time
import os
import sys
#import base64
import json
#import glob
import http
import http.client
import ssl
import argparse
import Messages_pb2

TOKEN_AUTH = None
FLAGS = None
def send_post (data, uri="/post/", method="POST", port="8000", host="127.0.0.1"):
    global FLAGS
    global TOKEN_AUTH
    try:
        h = http.client.HTTPConnection(host, port)
        headers = {}
        if TOKEN_AUTH is not None:
            headers['WWW-Authenticate'] = "Token"
            headers['Authorization'] = str("Token %s" % TOKEN_AUTH)
            headers['Content-Type'] = 'application/json'
        else:
            headers = {
                'Content-Type' : 'application/json',
            }
        h.request(method, uri, body=data, headers=headers)
        r = h.getresponse()
        if r.status != 200 and r.status != 201:
           raise Exception("Unable to post update ! (rc=%d)" % r.status)
        data = r.read()
        h.close()
        return data
    except:
        raise
    return "Unknown"

def main():
    tempbuf = []
    tempbuf2 = []
    ctx = zmq.Context()
    socket = ctx.socket(zmq.REP)
    socket.bind("tcp://*:5555")
    while True:
        message = socket.recv()
        gmsg = Messages_pb2.GenericMessage()
        gmsg.ParseFromString(message)
        next_sync = False
        if gmsg.msgtype == Messages_pb2.GenericMessage.MsgType.CONNECTION_NOTIFY:
            mcn = Messages_pb2.ConnectionNotify()
            mcn.ParseFromString(gmsg.data)
            
            ts = float(mcn.timestamp_s) + float(float(mcn.timestamp_us) / float(10**6))
            src = mcn.src
            dst = mcn.dst
            proto = mcn.protocol
            l4_src = mcn.l4_src
            l4_dst = mcn.l4_dst

            print("Connection@%.6f (proto=0x%02x, %s:%u -> %s:%u)" % (
                ts, proto, src, l4_src, dst, l4_dst
            ))

            d = {
                'hashstr' : mcn.hash,
                'timestamp_s' : mcn.timestamp_s,
                'timestamp_us' : mcn.timestamp_us,
                'protocol' : mcn.protocol,
                'src' : mcn.src,
                'dst' : mcn.dst,
                'l4_protocol': mcn.protocol,
                'l4_src': mcn.l4_src,
                'l4_dst': mcn.l4_dst,
                'msgtype': mcn.msgtype,
                'seqnum': mcn.seqnum,
                'state': 1
            }
            tempbuf += [d]
        elif gmsg.msgtype == Messages_pb2.GenericMessage.MsgType.CONNECTION_CLOSE_NOTIFY:
            mcn = Messages_pb2.ConnectionCloseNotify()
            mcn.ParseFromString(gmsg.data)
            
            ts = float(mcn.timestamp_s) + float(float(mcn.timestamp_us) / float(10**6))
            d = {
                'hashstr' : mcn.hash,
                'state': 2,
                'end_timestamp_s' : mcn.timestamp_s,
                'end_timestamp_us' : mcn.timestamp_us,                
            }
            tempbuf2 += [d]
        elif gmsg.msgtype == Messages_pb2.GenericMessage.MsgType.SYNC:
            next_sync = True

        if len(tempbuf) >= 100 or (next_sync and len(tempbuf) > 0):
            _d = json.dumps(tempbuf)
            actJson = send_post(_d, "/api/cmeta/", method="POST")
            tempbuf = []

        if len(tempbuf2) >= 100 or (next_sync and len(tempbuf2) > 0):
            for t in tempbuf2:
                _d = json.dumps([t])
                actJson = send_post(_d, "/api/cmeta/%s/" % (t['hashstr']), method="PUT")
                import pprint
                pprint.pprint(actJson)
                print(str(actJson))
            tempbuf2 = []

        socket.send(b"\x00")



def login ():
    global TOKEN_AUTH

    data = {
        'username' : FLAGS.username,
        'password' : FLAGS.password,
    }

    jsonData = json.dumps(data)
    tokenJson = send_post(jsonData, "/api/login/")
    TOKEN_AUTH = json.loads(tokenJson)['token']

    #print("Getting User list")
    #actJson = send_post(None, "/api/users/", method="GET")
    #actObj = json.loads(actJson)
    #import pprint
    #pprint.pprint(actObj)
    print(TOKEN_AUTH)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--username',
        type=str,
        default='admin'
    )

    parser.add_argument(
        '--password',
        type=str,
        default='admin'
    )

    parser.add_argument(
        '--host',
        type=str,
        default='127.0.0.1'
    )

    parser.add_argument(
        '--port', '-p',
        type=int,
        default=8082
    )

    FLAGS, unparsed = parser.parse_known_args()
    login()
    main()

