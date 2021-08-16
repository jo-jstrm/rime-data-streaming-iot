#!/bin/bash

#for fanout in ${fanouts[@]}; do
#  for ((i = 0; i < 11; i++)); do
#    # maintenance
#    sudo python3 build_logical_tree.py --log_tuples --fanout $fanout &&
#      sudo mkdir -p "/tmp/rime/logs/maintenance_run_${i}_fanout_${fanout}" &&
#      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/maintenance_run_${i}_fanout_${fanout}" &&
#      sudo rm -f /tmp/rime/logs/*.log
#  done
#done

movers=(7 1)
for mover in ${movers[@]}; do
  for ((i = 0; i < 11; i++)); do
    # movement
    # pick "movement" number of nodes to make movers
    # only makes sense on leaves
    sudo python3 build_logical_tree.py --movers $mover &&
      sudo mkdir -p "/tmp/rime/logs/movement_comparison_run_comparison_${i}_movers_${mover}" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/movement_comparison_run_comparison_${i}_movers_${mover}" &&
      sudo rm -f /tmp/rime/logs/*.log
  done
done

# remaining after already running on the server before
fanouts=(4 3 2)
for fanout in ${fanouts[@]}; do
  for ((i = 0; i < 11; i++)); do
    # selectivity 10
    sudo python3 build_logical_tree.py --send_query --query_x 90.0 --query_y 0.0 --fanout $fanout &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_10_run_${i}_fanout_${fanout}" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_10_run_${i}_fanout_${fanout}" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

  for ((i = 0; i < 11; i++)); do
    # selectivity 25
    sudo python3 build_logical_tree.py --send_query --query_x 50.0 --query_y 50.0 --fanout $fanout &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_25_run_${i}_fanout_${fanout}" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_25_run_${i}_fanout_${fanout}" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

  for ((i = 0; i < 11; i++)); do
    # selectivity 80
    sudo python3 build_logical_tree.py --send_query --query_x 20.0 --query_y 00.0 --fanout $fanout &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_80_run_${i}_fanout_${fanout}" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_80_run_${i}_fanout_${fanout}" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

#  for ((i = 0; i < 11; i++)); do
#    # failure recovery
#    # pick a parent node to kill from the middle depth
#    sudo python3 build_logical_tree.py --failure --fanout $fanout &&
#      sudo mkdir -p "/tmp/rime/logs/failure_recovery_run_${i}_fanout_${fanout}" &&
#      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/failure_recovery_run_${i}_fanout_${fanout}" &&
#      sudo rm -f /tmp/rime/logs/*.log
#  done

#  for ((i = 0; i < 11; i++)); do
#    # movement
#    # pick "movement" number of nodes to make movers
#    # only makes sense on leaves
#    sudo python3 build_logical_tree.py --movers 5 --fanout $fanout &&
#      sudo mkdir -p "/tmp/rime/logs/movement_run_${i}_fanout_${fanout}" &&
#      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/movement_run_${i}_fanout_${fanout}" &&
#      sudo rm -f /tmp/rime/logs/*.log
#  done

#  for ((i = 0; i < 11; i++)); do
#    # movement
#    # pick "movement" number of nodes to make movers
#    # only makes sense on leaves
#    sudo python3 build_logical_tree.py --movers 5 --log_tuples --fanout $fanout &&
#      sudo mkdir -p "/tmp/rime/logs/movement_log_tuples_run_${i}_fanout_${fanout}" &&
#      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/movement_log_tuples_run_${i}_fanout_${fanout}" &&
#      sudo rm -f /tmp/rime/logs/*.log
#  done

#  for ((i = 0; i < 11; i++)); do
#    # base line / line-rate
#    sudo python3 build_logical_tree.py --baseline "74797469/rime-baseline:latest" --fanout $fanout &&
#      sudo mkdir -p "/tmp/rime/logs/baseline_500_run_${i}_fanout_${fanout}" &&
#      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/baseline_500_run_${i}_fanout_${fanout}" &&
#      sudo rm -f /tmp/rime/logs/*.log
#  done
done

fanouts=(3 2)
for fanout in ${fanouts[@]}; do
  for ((i = 0; i < 11; i++)); do
    # maintenance
    sudo python3 build_logical_tree.py --log_tuples --fanout $fanout --depth 3 &&
      sudo mkdir -p "/tmp/rime/logs/maintenance_run_${i}_fanout_${fanout}_height_3" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/maintenance_run_${i}_fanout_${fanout}_height_3" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

  for ((i = 0; i < 11; i++)); do
    # selectivity 10
    sudo python3 build_logical_tree.py --send_query --query_x 90.0 --query_y 0.0 --fanout $fanout --depth 3 &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_10_run_${i}_fanout_${fanout}_height_3" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_10_run_${i}_fanout_${fanout}_height_3" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

  for ((i = 0; i < 11; i++)); do
    # selectivity 25
    sudo python3 build_logical_tree.py --send_query --query_x 50.0 --query_y 50.0 --fanout $fanout --depth 3 &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_25_run_${i}_fanout_${fanout}_height_3" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_25_run_${i}_fanout_${fanout}_height_3" &&
      sudo rm -f /tmp/rime/logs/*.log
  done

  for ((i = 0; i < 11; i++)); do
    # selectivity 80
    sudo python3 build_logical_tree.py --send_query --query_x 20.0 --query_y 00.0 --fanout $fanout --depth 3 &&
      sudo mkdir -p "/tmp/rime/logs/selectivity_80_run_${i}_fanout_${fanout}_height_3" &&
      sudo cp -f /tmp/rime/logs/*.log "/tmp/rime/logs/selectivity_80_run_${i}_fanout_${fanout}_height_3" &&
      sudo rm -f /tmp/rime/logs/*.log
  done
done