---
title = "Getting Started"
slug = "docs"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Getting Started"
weight = 1
lead = "Install Toucan and build your first handler in under five minutes."
---

## Requirements

You need [Donna](https://donna-lang.org/docs/installation/) installed. Create a new project:

```sh
donna new my-app
cd my-app
```

## Install

Add Toucan to your `donna.toml`:

```toml
[dependencies]
donna = { git = "https://github.com/donna-lang/donna_stdlib", version = ">=0.2.0 and <1.0.0" }
toucan = { git = "https://github.com/NikolasSkyl/toucan", version = ">=0.1.0 and <1.0.0" }
```

## Hello World

Replace `src/my_app.donna` with:

```donna
import toucan

fn handler(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.landing_page
  |> toucan.not_found

fn home(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/hello")
  |> toucan.html("<h1>Hello from Toucan!</h1>")

pub fn main() -> Nil:
  toucan.serve(handler, 8080)
```

Run it:

```sh
donna run
```

Visit `http://localhost:8080` — you'll see the Toucan landing page. Add a `GET /` route and it will replace it.

## How it works

A **Ctx** (context) flows through your handler pipeline. Each function either:

- **Passes** — modifies the `Ctx` and returns it as `Active` or `Matched`
- **Rejects** — returns `Rejected`, causing `try` to restore the previous active state and continue

This means routing is just function composition. There is no router object, no registration step, no magic string matching — only functions piped together.

## Next steps

- [Routing](/docs/routing/) — path patterns and method filters
- [Handlers](/docs/handlers/) — writing and composing handlers
- [Requests](/docs/requests/) — reading headers, body, query, and form data
- [Responses](/docs/responses/) — HTML, JSON, redirects, and custom status codes
- [WebSockets](/docs/websockets/) — real-time apps with `serve_ws` and `serve_chat`
- [Middleware](/docs/middleware/) — CORS, CSP, security headers, static files

## Examples

The repository includes runnable apps in `examples/`. They cover the common paths:
basic routing, SQLite-backed pages, JSON APIs, login sessions, file/image upload,
URL shortening, htmx fragments, and WebSocket chat with named users.

Run any example from its directory:

```sh
cd examples/basic
donna run
```

All examples listen on `http://localhost:8000`.
