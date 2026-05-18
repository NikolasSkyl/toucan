---
title = "Middleware"
slug = "docs/middleware"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 60
lead = "CORS, CSP, security headers, logging, static files, and writing your own middleware."
---

## Security headers

`toucan.secure_headers` adds `X-Content-Type-Options`, `X-Frame-Options`, and `Referrer-Policy` to every matched response. Toucan applies these automatically on every response dispatched by `serve` — you only need to call this directly in unusual pipelines.

## CORS

Handle cross-origin requests with `toucan.cors` and `toucan.preflight`:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.preflight
  |> toucan.try(api_data)
  |> toucan.not_found
  |> toucan.cors("https://myapp.com")
```

- `preflight` intercepts `OPTIONS` requests and responds 204
- `cors(c, origin)` adds `Access-Control-Allow-Origin`, `Allow-Methods`, and `Allow-Headers` to any matched response

Pass `"*"` to allow all origins:

```donna
  |> toucan.cors("*")
```

## Content Security Policy

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.not_found
  |> toucan.csp("default-src 'self'; img-src 'self' data:")
```

`toucan.csp(c, policy)` adds `Content-Security-Policy: <policy>` to every matched response.

## Request logging

Use `serve_logged` instead of `serve` to print coloured request lines:

```donna
pub fn main() -> Nil:
  toucan.serve_logged(router, 8080, logger.Verbose)
```

Or with shared state:

```donna
pub fn main() -> Nil:
  toucan.serve_with_logged(db, router, 8080, logger.Verbose)
```

Pass `logger.Silent` to suppress output entirely.

## Static files

`toucan.static_dir(c, dir)` maps the request path to a file inside `dir`. Returns `Rejected` when the file is not found so the next handler can run.

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(serve_assets)
  |> toucan.try(home)
  |> toucan.not_found

fn serve_assets(c: toucan.Ctx) -> toucan.Ctx:
  toucan.static_dir(c, "priv/static")
```

A request to `/style.css` looks for `priv/static/style.css`. Paths ending in `/` resolve to `index.html`.

## Custom middleware

Any `fn(Ctx) -> Ctx` is middleware. The three Ctx variants let you decide when to act:

- `Active` — the request is still being matched, no response yet
- `Rejected` — a filter didn't match; usually pass it through unchanged
- `Matched` — a response has been set; this is when you add headers or log

### Adding a header to every response

The most common pattern: do nothing until a response is matched, then decorate it:

```donna
fn cache_control(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.set_header("Cache-Control", "no-store")
```

Apply it after the rest of the pipeline so it always runs on the final response:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(home)
  |> toucan.try(api_data)
  |> toucan.not_found
  |> cache_control
```

### Auth middleware

Inspect the request while `Active`, reject early if the check fails:

```donna
fn require_auth(c: toucan.Ctx) -> toucan.Ctx:
  case c:
    toucan.Active(_, _) ->
      case toucan.header_of(c, "Authorization"):
        "" -> toucan.respond(c, 401, "text/plain", "Unauthorized")
        _  -> c
    other -> other
```

Wrap only the routes that need protection with `toucan.try`:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(public_home)
  |> toucan.try(protected_route)
  |> toucan.not_found

fn protected_route(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/admin")
  |> require_auth
  |> toucan.html(admin_html())
```

### Shared state middleware

When using `serve_with`, thread state through custom middleware using `try_with`:

```donna
fn require_user(c: toucan.Ctx, db: Db) -> toucan.Ctx:
  case c:
    toucan.Active(_, _) ->
      let token = toucan.header_of(c, "Authorization")
      case db_lookup_token(db, token):
        False -> toucan.respond(c, 403, "text/plain", "Forbidden")
        True  -> c
    other -> other

fn protected(c: toucan.Ctx, db: Db) -> toucan.Ctx:
  c
  |> toucan.try_with(db, require_user)
  |> toucan.get
  |> toucan.path("/dashboard")
  |> toucan.html(dashboard_html())
```
