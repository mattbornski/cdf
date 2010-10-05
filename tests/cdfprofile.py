#!/usr/bin/env python

import cdf
import cherrypy
import dowser
import sys
import time

def start(port):
    cherrypy.tree.mount(dowser.Root())
    cherrypy.config.update({
        'environment': 'embedded',
        'server.socket_port': port
    })
    cherrypy.engine.start()

if __name__ == '__main__':
    start(8000)
    begin = time.time()
    a = cdf.archive(sys.argv[1])
    end = time.time()
    print 'time elapsed: ' + str(end - begin) + ' seconds'
    input()
