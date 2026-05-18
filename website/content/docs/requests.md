---
title = "Requests"
slug = "docs/requests"
date = "2026-05-18"
layout = "doc"
draft = false
category = "Core"
weight = 30
lead = "Read headers, body, query strings, and form data from the incoming request."
---

## Method and path

```donna
let method = toucan.method_of(c)   // "GET", "POST", ...
let path   = toucan.path_of(c)     // "/users/42"
let query  = toucan.query_of(c)    // "page=2&sort=asc"
```

## Headers

Read a specific header by name (case-insensitive):

```donna
let auth   = toucan.header_of(c, "Authorization")
let cookie = toucan.header_of(c, "Cookie")
let ct     = toucan.content_type_of(c)
```

Returns `""` when the header is absent.

## Request body

```donna
let body = toucan.body_of(c)
```

The raw body string. For JSON APIs, parse this directly. For forms, use `form_field` instead.

## Query parameters

Parse the query string into key-value pairs:

```donna
let params = toucan.query_params(c)           // List(#(String, String))
let page   = toucan.query_param(c, "page")    // "2" or ""
let sort   = toucan.query_param(c, "sort")    // "asc" or ""
```

## Form data

Parse `application/x-www-form-urlencoded` request bodies:

```donna
fn handle_signup(c: toucan.Ctx) -> toucan.Ctx:
  let name  = toucan.form_field(c, "name")
  let email = toucan.form_field(c, "email")
  case string.is_empty(name):
    True  -> toucan.redirect(c, "/signup?error=missing_name")
    False -> create_user_and_redirect(c, name, email)
```

`form_field` returns `""` when the field is absent or the body is not form-encoded.

For large multipart uploads, prefer a focused parser in your app or a package
designed for binary uploads. The `file_upload` example shows the current shape:
small text uploads can be parsed in Donna, while image uploads use a small FFI
helper so files are written as real binary data.

## URL parameters

Parameters captured from `:name` segments in `toucan.path`:

```donna
fn show_user(c: toucan.Ctx) -> toucan.Ctx:
  c
  |> toucan.get
  |> toucan.path("/users/:id")
  |> render_user

fn render_user(c: toucan.Ctx) -> toucan.Ctx:
  let id = toucan.param(c, "id")
  toucan.json(c, "{\"id\":\"" <> id <> "\"}")
```
