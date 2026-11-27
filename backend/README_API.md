# CAMPuVISE API

This local API serves the frontend and stores reports in `reports.txt`.

## Build and run

Requires C++17, cpp-httplib, CMake, and pthreads.

```bash
cmake -S . -B build
cmake --build build
./build/campuvise-server
```

The server listens on `127.0.0.1:8080`; opening `/` serves the frontend.

## Report routes

- `GET /api/reports` returns the server report list.
- `POST /api/reports` accepts multipart fields `location`, `floor`, `room`, `landmark`, `wasteType`, `severity`, `description`, and `reportedBy`, plus optional image file `image`. `isDemo` is optional. It returns the created report with its `CV-<number>` ID.
- `POST /api/reports/assign` accepts `id`, `worker`, and `actor`.
- `POST /api/reports/<id>/start` accepts `actor`.
- `POST /api/reports/<id>/complete` accepts `actor`, `note`, and optional `beforeImage` and `afterImage` files.
- `POST /api/reports/<id>/approve` accepts `actor`.
- `POST /api/reports/<id>/rework` accepts `actor` and `reason`.
- `POST /api/reports/delete-all` removes all reports.
- `DELETE /api/reports/<id>` removes one report.
- Compatibility routes `POST /api/reports/verify` and `POST /api/reports/complete` remain available for the original API clients.
- `GET /api/images/<id>`, `GET /api/images/<id>/before`, and `GET /api/images/<id>/after` serve uploaded photos.

Report statuses match the frontend: `Submitted`, `Assigned`, `In Progress`, `Awaiting Verification`, and `Resolved`. The versioned, percent-encoded pipe-separated store reads earlier six-to-eight-column rows. Its first ID, status, and worker columns remain readable by the existing console tools.

The role login remains a frontend demo and is not server authentication. The API binds to loopback and has no authorization or production-grade concurrency protection; keep it on a trusted local machine.
