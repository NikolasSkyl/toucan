---
title = "Responses"
slug = "docs/responses"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 40
lead = "Send HTML, JSON, redirects, files, and custom responses."
---

## Text and HTML

```donna
toucan.ok(c, "plain text")
toucan.html(c, "<h1>Hello</h1>")
```

Both respond 200. `ok` uses `text/plain; charset=utf-8`, `html` uses `text/html; charset=utf-8`.

## JSON

```donna
toucan.json(c, "{\"status\":\"ok\"}")
```

Sets status 200 and `Content-Type: application/json`. The body is passed as-is — build your JSON string in Donna or use a JSON library.

## Status codes

```donna
toucan.created(c, "user created")   // 201
toucan.no_content(c)                // 204
toucan.respond(c, 422, "text/plain; charset=utf-8", "validation error")
```

## Redirects

```donna
toucan.redirect(c, "/dashboard")
```

Responds 302 with a `Location` header. Use after a form POST to follow the PRG pattern.

## Response headers

Chain `set_header` after any terminal function:

```donna
fn download(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/export")
  |> toucan.respond(200, "text/csv; charset=utf-8", csv_data())
  |> toucan.set_header("Content-Disposition", "attachment; filename=\"export.csv\"")
```

## Static files

Serve files from a directory. The request path is mapped to a file inside `dir`. Directories resolve to `index.html`. Returns `Rejected` when the file does not exist, so subsequent handlers can still match.

```donna
fn serve_static(c: toucan.Ctx) -> toucan.Ctx:
  toucan.static_dir(c, "priv/static")
```

Place it early in the pipeline:

```donna
fn router(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.try(serve_static)
  |> toucan.try(home)
  |> toucan.not_found
```
