#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

def toscgi(text):
    def to_headers():
        res = []
        hdr = text[:text.find(b'\n\n')]
        for element in hdr.split(b'\n'):
            [key,value]=element.split(b':')
            res.append((key.upper().replace(b'-',b'_'),value))
        return res
    def to_body():
        return text[text.find(b'\n\n')+2:]
    def format_headers(headers):
        result = b''
        for [key,value] in headers:
            result+=key+b'\0'+value+b'\0'
        return result
    headers=to_headers()
    body=to_body()
    headers = [(b'CONTENT_LENGTH',str(len(body)).encode())] + headers
    headers=format_headers(headers)
    return str(len(headers)).encode()+b':' + headers +b',' + body
