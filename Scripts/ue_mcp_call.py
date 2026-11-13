# Talks to the editor's Unreal MCP server over plain HTTP, for when a Claude Code session started
# before the editor and its unreal-mcp connection failed. Same server, same tools.
# Calls one Unreal MCP toolset tool over HTTP: ue_mcp_call.py <toolset> <tool> '<json args>'
# Prints the tool's text result; images (base64) are written to the PNG path given with -o.
import json, sys, urllib.request, base64
URL = "http://127.0.0.1:8000/mcp"
H = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
def rpc(method, params, sid=None, i=1):
    h = dict(H)
    if sid: h["Mcp-Session-Id"] = sid
    req = urllib.request.Request(URL, json.dumps({"jsonrpc": "2.0", "id": i, "method": method, "params": params}).encode(), h)
    with urllib.request.urlopen(req, timeout=115) as r:
        return r.headers.get("Mcp-Session-Id"), r.read().decode()
args = sys.argv[1:]
out = None
if "-o" in args:
    k = args.index("-o"); out = args[k + 1]; del args[k:k + 2]
toolset, tool = args[0], args[1]
targs = json.loads(args[2]) if len(args) > 2 else {}
sid, _ = rpc("initialize", {"protocolVersion": "2025-03-26", "capabilities": {}, "clientInfo": {"name": "tribute-cli", "version": "1"}})
p = {"tool_name": tool, "arguments": targs}
if toolset != "-": p["toolset_name"] = toolset
_, body = rpc("tools/call", {"name": "call_tool", "arguments": p}, sid, 2)
d = json.loads(body)
if "error" in d: print("RPC ERROR", json.dumps(d["error"])); sys.exit(1)
for c in d["result"].get("content", []):
    if c.get("type") == "text":
        t = c["text"]
        if out and '"data"' in t and len(t) > 100000:
            j = json.loads(t); img = j["returnValue"]["image"] if "image" in j["returnValue"] else j["returnValue"]
            open(out, "wb").write(base64.b64decode(img["data"])); print("image ->", out)
            rest = {k: v for k, v in j["returnValue"].items() if k != "image"} if "image" in j["returnValue"] else {}
            print(json.dumps(rest)[:2000])
        else:
            print(t[:20000])
    elif c.get("type") == "image" and out:
        open(out, "wb").write(base64.b64decode(c["data"])); print("image ->", out)
if d["result"].get("isError"): sys.exit(2)
