#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

def toscgi(text):
    def to_headers():
        res = []
        hdr = text[:text.find('\n\n')]
        for element in hrd.split('\n'):
            [key,value]=element.split(':')
            res.append((key.upper().replace('-','_'),value))
        return res
    def to_body()
        return text[text.find('\n\n')+2:]
    def format_headers(headers):
        result = ''
        for [key,value] in headers:
            result+=key+'\0'+value+'\0'
        return result
    headers=to_headers(text)
    body=to_body(text)
    headers = [('CONTENT_LENGTH',len(body))] + headers
    headers=format_headers(headers)
    return str(len(headers))+':' + headers +',' + body
