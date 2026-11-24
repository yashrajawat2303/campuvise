
# CAMPuVISE

CAMPuVISE is a student-focused management tool built using C++ to organize and manage student-related records.

## Features

- Student record management
- Record storage using text files
- Record cleanup functionality
- Standalone frontend interface

## Project Structure

```text
campuvise/
├── backend/
│   ├── main.cpp
│   ├── cleanup.cpp
│   ├── cleanup.h
│   ├── reports.cpp
│   ├── reports.h
│   ├── reports.txt
│   └── next_id.txt
└── frontend/
    └── campuvise-standalone.html
```

## Technologies

- C++
- HTML (standalone frontend)

## Running the Backend

```bash
cd backend
g++ cleanup.cpp main.cpp reports.cpp -o campusvise
./campusvise
```

## Status

Currently under development.

## Feedback & Contributions

Have suggestions, found a bug, or want to improve CAMPuVISE?

Feel free to open an Issue on GitHub. Feedback and suggestions are welcome!
