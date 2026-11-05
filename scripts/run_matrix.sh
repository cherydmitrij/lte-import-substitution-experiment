#!/usr/bin/env bash
echo "seed,load,cfg,throughput_Mbps,ue_with_traffic"
for SEED in 1000 2000 3000 4000 5000; do
  for LOAD in low med high; do
    for CFG in foreign domestic; do
      OUT=$(./ns3 run "scratch/my-lte-5g --seed=${SEED} --load=${LOAD} --enbConfig=${CFG} --simTime=60" 2>&1)
      TP=$(echo "$OUT" | grep "Throughput(Mbps)=" | cut -d= -f2 | tr -d ' ')
      UE=$(echo "$OUT" | grep "UE_with_traffic=" | cut -d= -f2 | tr -d ' ')
      echo "${SEED},${LOAD},${CFG},${TP},${UE}"
    done
  done
done
