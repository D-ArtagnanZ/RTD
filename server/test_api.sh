#!/bin/bash

HOST="http://localhost:8080"

# 健康检查
echo "测试健康检查..."
curl -s $HOST/health

echo -e "\n\n测试设备查询..."
curl -s -X POST $HOST/api/getEqpById \
  -H "Content-Type: application/json" \
  -d '{"id": "EQP001"}' | jq

echo -e "\n\n测试批次查询..."
curl -s -X POST $HOST/api/getLotById \
  -H "Content-Type: application/json" \
  -d '{"id": "LOT001"}' | jq

echo -e "\n\n测试设备列表（分页）..."
curl -s -X POST $HOST/api/getEqpList \
  -H "Content-Type: application/json" \
  -d '{"page_no": 1, "page_size": 10}' | jq

echo -e "\n\n测试批次列表（分页）..."
curl -s -X POST $HOST/api/getLotList \
  -H "Content-Type: application/json" \
  -d '{"page_no": 1, "page_size": 10}' | jq

echo -e "\n\n测试调度列表（分页）..."
curl -s -X POST $HOST/api/getDispatchList \
  -H "Content-Type: application/json" \
  -d '{"page_no": 1, "page_size": 10}' | jq

echo -e "\n\n测试根据设备ID查询调度记录..."
curl -s -X POST $HOST/api/searchDispatchByEqpId \
  -H "Content-Type: application/json" \
  -d '{"eqp_id": "EQP001"}' | jq
