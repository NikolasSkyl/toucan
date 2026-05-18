---
title = "WebSockets"
slug = "docs/websockets"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 50
lead = "Real-time connections with serve_ws, serve_chat, and serve_with_chat."
---

## serve_ws

Use `serve_ws` when you want to echo or individually reply to WebSocket messages. The `ws` callback receives the connection ID and message text, and returns the reply. Return `""` to send nothing.

```donna
import toucan

fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(ws_page)
  |> toucan.not_found

fn ws_page(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/echo")
  |> toucan.html(echo_html())

fn on_message(conn_id: String, msg: String) -> String:
  msg   // echo back to sender

pub fn main() -> Nil:
  toucan.serve_ws(router, on_message, 8080)
```

HTTP requests are handled exactly as they would be with `toucan.serve` — the `ws` callback is only called for WebSocket messages.

## serve_chat

Use `serve_chat` when you want to broadcast messages to all connected clients. On connect, each client automatically receives `"#" <> conn_id` so it knows its own ID. The return value of the `ws` callback is broadcast to everyone.

```donna
fn on_message(conn_id: String, msg: String) -> String:
  conn_id <> "|" <> msg   // broadcast "sender_id|text"

pub fn main() -> Nil:
  toucan.serve_chat(router, on_message, 8080)
```

## serve_with_chat

Use `serve_with_chat` when your HTTP routes need shared state and your WebSocket
callback should run in the same server. The `chat_sessions` example uses this to
combine cookie sessions with a named chat room:

```donna
import toucan
import toucan/session

fn router(c: toucan.Ctx, store: session.Store) -> toucan.Ctx:
  c
  |> toucan.try_with(store, join_page)
  |> toucan.try_with(store, chat_page)
  |> toucan.not_found

fn on_message(conn_id: String, msg: String) -> String:
  msg

pub fn main() -> Nil:
  toucan.serve_with_chat(session.new_store(), router, on_message, 8080)
```

### Client-side ID detection

The server sends `"#N"` immediately after the WebSocket handshake. Parse it to know your own ID:

```js
let myId = null;
ws.onmessage = e => {
  if (e.data.startsWith('#')) {
    myId = e.data.slice(1);
    return;
  }
  const sep = e.data.indexOf('|');
  const sender = e.data.slice(0, sep);
  const text   = e.data.slice(sep + 1);
  display(text, sender === myId ? 'mine' : 'theirs');
};
```

## WebSocket events

Both `serve_ws` and `serve_chat` expose the underlying mongoose event types through the `mongoose` package if you need lower-level access. The `mongoose.accept_event` function returns an `Event` that is one of:

| Variant | Fields | Description |
|---|---|---|
| `Http(req)` | request | An HTTP request |
| `WsOpen(conn_id, path)` | id, path | New WebSocket connection |
| `WsMsg(conn_id, data)` | id, text | Incoming message |
| `WsClose(conn_id)` | id | Connection closed |

## Connection IDs

Connection IDs are monotonically increasing decimal integers (`"1"`, `"2"`, …).They are unique per server lifetime and never reused after a connection closes.
