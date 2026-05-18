---
title = "Handlers"
slug = "docs/handlers"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 20
lead = "Handlers are plain functions. Compose them freely."
---

## The Ctx type

Every handler receives a `Ctx` and returns a `Ctx`. The context is one of three states:

- `Active(req, params)` — request is still being matched
- `Rejected(req)` — a filter did not match; `try` restores `Active`
- `Matched(res)` — a response has been set; all subsequent handlers are skipped

You never construct these directly. Toucan's filter functions return the right state automatically.

## Writing a handler

A handler is any function with the signature `fn(Ctx) -> Ctx`:

```donna
fn hello(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/hello")
  |> toucan.html("<h1>Hello!</h1>")
```

If `toucan.get` finds the method is not GET it returns `Rejected`. `toucan.path` does the same for a mismatched path. Neither `toucan.html` nor any further handler runs because `|>` still calls the function — but terminal functions like `toucan.html` check the state first and skip themselves when `Rejected`.

## try and try_with

Wrap handlers in `toucan.try` to make mismatches non-fatal:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(hello)
  |> toucan.try(about)
  |> toucan.not_found
```

When `hello` rejects, `try` restores the `Active` state so `about` can attempt its own filters.

For handlers that need extra data, use `try_with`:

```donna
fn router(c: toucan.Ctx, cfg: Config) -> toucan.Ctx:
  c
  |> toucan.try_with(cfg, dashboard)
  |> toucan.not_found
```

## Terminal functions

Terminal functions set the response and return `Matched`. They only act when the context is `Active`:

| Function | Status | Content-Type |
|---|---|---|
| `toucan.ok(c, body)` | 200 | `text/plain` |
| `toucan.html(c, body)` | 200 | `text/html` |
| `toucan.json(c, body)` | 200 | `application/json` |
| `toucan.created(c, body)` | 201 | `text/plain` |
| `toucan.no_content(c)` | 204 | — |
| `toucan.redirect(c, url)` | 302 | — |
| `toucan.respond(c, status, ct, body)` | any | any |

## Adding response headers

Use `toucan.set_header` after a terminal function:

```donna
fn api_data(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/data")
  |> toucan.json("{\"ok\":true}")
  |> toucan.set_header("Cache-Control", "no-store")
```

## not_found and internal_error

Place `toucan.not_found` at the end of a router to respond 404 when nothing matched:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.not_found
```

`toucan.internal_error` serves a styled 500 page — useful as a fallback after error handling logic.

## landing_page

`toucan.landing_page` serves the built-in Toucan welcome page for `GET /`. Place it before `not_found` during development so you see something useful before you wire up your root route:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.landing_page
  |> toucan.not_found
```
