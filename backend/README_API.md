# CAMPuVISE API (starter)

This is an initial local API layer around the text-file report store. Existing console files are preserved.

## Dependencies

- C++17 compiler
- cpp-httplib header (`httplib.h`), available via package manager
- Linux: CMake and pthreads

On Arch Linux, install: `sudo pacman -S cmake gcc cpp-httplib`

## Build

Copy/install `httplib.h` so the compiler can find it, then:

```bash
cmake -S . -B build
cmake --build build
./build/campuvise-server
```

Server binds to localhost port 8080.

## Endpoints
- `GET /api/reports`
- `POST /api/reports` form fields: `location`, `wasteType`, `severity`, `description`, optional multipart `image`
- `POST /api/reports/verify` form field `id`
- `POST /api/reports/assign` form fields `id`, `worker`
- `POST /api/reports/complete` form field `id`
- `DELETE /api/reports/<id>`
- `GET /api/images/<id>`

Workflow: Pending -> Verified -> Assigned -> Completed.

## Notes
This is a classroom/demo starter, not a production-ready service. It binds only to loopback and has no authentication, authorization, rate limits, or robust validation yet. Do not expose it to the internet or use real personal data. Text file writes are not concurrency-safe. Image uploads are limited by request size and extension, but MIME/content verification should be added before production use.
