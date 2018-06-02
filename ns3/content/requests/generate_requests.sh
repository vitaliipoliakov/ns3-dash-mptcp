#!/bin/bash

echo "UserId,StartsAt,StopsAt,VideoId,LinkCapacity,ScreenWidth,ScreenHeight" > requests

for i in {0..100}
do
	UserId=$i
	StartAt=$(($UserId+1))
	StopAt=$(($UserId+10))
	echo "$UserId,$StartAt,$StopAt,1,3337,1280,720" >> requests
done
