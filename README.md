# Personal Notes

A personal notes and portfolio site built with [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/).

## Serve locally

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
mkdocs serve
```

## Deploy

```bash
mkdocs build
```

This generates a static site in `site/`, which can be deployed to any static hosting provider (e.g. GitHub Pages via `mkdocs gh-deploy`).
