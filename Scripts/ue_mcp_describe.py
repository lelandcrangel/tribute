# Talks to the editor's Unreal MCP server over plain HTTP, for when a Claude Code session started
# before the editor and its unreal-mcp connection failed. Same server, same tools.
# Lists a toolset's tools: ue_mcp_describe.py <toolset> [tool-name-fragment for the full schema]
import json, sys, urllib.request
URL="http://127.0.0.1:8000/mcp"; H={"Content-Type":"application/json","Accept":"application/json, text/event-stream"}
SID=[None]
def rpc(m,p,i=1):
    h=dict(H)
    if SID[0]: h["Mcp-Session-Id"]=SID[0]
    r=urllib.request.Request(URL,json.dumps({"jsonrpc":"2.0","id":i,"method":m,"params":p}).encode(),h)
    resp=urllib.request.urlopen(r,timeout=60)
    SID[0]=SID[0] or resp.headers.get("Mcp-Session-Id")
    return json.loads(resp.read().decode())
rpc("initialize",{"protocolVersion":"2025-03-26","capabilities":{},"clientInfo":{"name":"x","version":"1"}})
d=rpc("tools/call",{"name":"describe_toolset","arguments":{"toolset_name":sys.argv[1]}},2)
t=json.loads(d["result"]["content"][0]["text"])
full = len(sys.argv) > 2
for tool in t["tools"]:
    if full and sys.argv[2] in tool["name"]:
        print(json.dumps(tool, indent=1)[:6000])
    elif not full:
        print(tool["name"].split(".")[-1], "-", tool["description"].strip().splitlines()[0][:110])
