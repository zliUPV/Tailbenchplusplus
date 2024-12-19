#!/usr/bin/python

import sys
import os
import numpy as np
from scipy import stats

class Lat(object):
    def __init__(self, fileName):
        f = open(fileName, 'rb')
        a = np.fromfile(f, dtype=np.uint64)
        self.reqTimes = a.reshape((a.shape[0]/5, 5))
        f.close()

    def parseQueueTimes(self):
        return self.reqTimes[:, 0]

    def parseSvcTimes(self):
        return self.reqTimes[:, 1]

    def parseSojournTimes(self):
        return self.reqTimes[:, 2]

    def parseRequestDate(self):
        return self.reqTimes[:, 3]

    def parseNicSojournTimes(self):
        return self.reqTimes[:, 4]


if __name__ == '__main__':
    def getLatPct(latsFile):
        assert os.path.exists(latsFile)

        latsObj = Lat(latsFile)

        qTimes = [l/1e6 for l in latsObj.parseQueueTimes()]
        svcTimes = [l/1e6 for l in latsObj.parseSvcTimes()]
        sjrnTimes = [l/1e6 for l in latsObj.parseSojournTimes()]
        reqDate = [l for l in latsObj.parseRequestDate()]
        NicSjrnTimes = [l/1e6 for l in latsObj.parseNicSojournTimes()]

        f = open('lats.txt','w')

        f.write('%12s %12s %12s %12s %12s\n\n' \
                % ('QueueTimes', 'ServiceTimes', 'SojournTimes', 'RequestDate(usec)', 'NicSojournTimes'))

        for (q, svc, sjrn, rdate, nicsjrn) in zip(qTimes, svcTimes, sjrnTimes, reqDate, NicSjrnTimes):
            f.write("%12s %12s %12s %12s %12s\n" \
                    % ('%.3f' % q, '%.3f' % svc, '%.3f' % sjrn, '%.0f' % rdate, '%.3f' % nicsjrn))
        f.close()
        p95 = stats.scoreatpercentile(sjrnTimes, 95)
        maxLat = max(sjrnTimes)
        print "95th percentile latency %.3f ms | max latency %.3f ms" \
                % (p95, maxLat)

    latsFile = sys.argv[1]
    getLatPct(latsFile)
        
