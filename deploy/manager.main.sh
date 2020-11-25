#!/bin/bash

root_dir=`dirname $( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )`

source "${root_dir}/deploy/python.rc"
clear;

echo $root_dir
echo ">Enter project":
echo ">1. Satellite"
echo ">2. Telemetry"
echo ">3. Military"
read prj
echo "Project ${prj}"


if [[ $prj -eq 1 ]]
then
  python ./topo/distributed/main.py \
  --config "${root_dir}/topo/distributed/satellite.config.json" \
  --topos_fn "${root_dir}/static/satellite_overall.pkl"
elif [[ $prj -eq 2 ]]
then
   python ./topo/distributed/main.py \
  --config "${root_dir}/topo/distributed/telemetry.config.json" \
  --topos_fn "${root_dir}/static/satellite_overall.pkl"
elif [[ $prj -eq 3 ]]
then
       python ./topo/distributed/main.py \
  --config "${root_dir}/topo/distributed/military.config.json" \
  --topos_fn "${root_dir}/static/military.pkl"

else
  echo "Invalid project"
fi