#!/bin/sh

# $Id: exp.sh 4285 2009-02-19 19:02:52Z rares $
#
# Copyright (C) 2007 by The Regents of the University of California
#
# Redistribution of this file is permitted under the terms of the BSD
# license
#
# Date: 08/28/2008
#
# Author: Rares Vernica <rares (at) ics.uci.edu>
#

for e in 1 2 3 4
do
    ./perftest $e > exps/xDataSize_yTime_zAlg/imdb/wh/jacc/$e
done
