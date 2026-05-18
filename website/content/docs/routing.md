---
title = "Routing"
slug = "docs/routing"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 10
lead = "Match requests by method and path. Capture URL parameters."
---

## Method filters

Use `toucan.get`, `toucan.post`, `toucan.put`, `toucan.delete`, or `toucan.patch` to match an HTTP method. If the request method doesn't match, the context becomes `Rejected` and `try` moves on.

```donna
fn update_user(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.put
  |> toucan.path("/users/:id")
  |> handle_update
```

## Path matching

`toucan.path(c, pattern)` matches the request path against a pattern:

- **Literal segments** — `/about`, `/api/v1/status`
- **Parameter segments** — `:name` captures one path segment
- **Exact match** — the pattern must match the entire path, not just a prefix

```donna
fn show_post(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/posts/:slug")
  |> render_post
```

## URL parameters

Read captured segments with `toucan.param(c, "name")`. Returns `""` if the key was not captured.

```donna
fn render_post(c: toucan.Ctx) -> toucan.Ctx:
  let slug = toucan.param(c, "slug")
  toucan.html(c, "<h1>" <> slug <> "</h1>")
```

## Path prefixes

Use `toucan.path_prefix(c, prefix)` to match any path that starts with a prefix — useful for grouping routes under a sub-path:

```donna
fn api_routes(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.path_prefix("/api")
  |> toucan.try(api_users)
  |> toucan.try(api_posts)
```

## Composing routes

Wrap each route in a function and pipe them through `toucan.try`. The first handler to return a `Matched` context wins; all subsequent handlers are skipped.

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.try(about)
  |> toucan.try(show_user)
  |> toucan.try(create_user)
  |> toucan.not_found
```

## Shared state

When handlers need access to a database or configuration, use `toucan.try_with` and `toucan.serve_with`:

```donna
fn router(c: toucan.Ctx, db: sqlite.Db) -> toucan.Ctx:
  c
  |> toucan.try_with(db, list_users)
  |> toucan.try_with(db, create_user)
  |> toucan.not_found

pub fn main() -> Nil:
  let db = sqlite.open("app.db")
  toucan.serve_with(db, router, 8080)
```

Each handler receives both the context and the shared state:

```donna
fn list_users(c: toucan.Ctx, db: sqlite.Db) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/users")
  |> render_users(db)
```

## Preflight (OPTIONS)

For CORS preflight requests, place `toucan.preflight` at the top of the pipeline:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.preflight
  |> toucan.try(home)
  |> toucan.not_found
  |> toucan.cors("*")
```

`preflight` intercepts `OPTIONS` requests and responds 204. The `cors` call at the end adds the CORS headers to every matched response.
