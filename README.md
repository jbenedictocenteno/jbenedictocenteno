# Personal Notes

Hi, I'm Jorge Benedicto Centeno, a Spanish electronic engineer specialized in embedded systems. I've designed hardware and firmware for IoT devices, and I'm now branching out into other areas as well. This repo holds the source for my personal notes site: doubts, tech research, and design notes for the public projects I work on.

📖 **[Read the docs](https://jbenedictocenteno.github.io/jbenedictocenteno/)**

Built with [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/).

## Contact

If you need to reach me, feel free to email me at [jorgebene_dicto@yahoo.es](mailto:jorgebene_dicto@yahoo.es).

## Serve locally

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
mkdocs serve
```

## Deploy

Every push to `main` triggers [`.github/workflows/deploy.yml`](.github/workflows/deploy.yml), which builds the site and publishes it to the `gh-pages` branch automatically.

To deploy manually instead:

```bash
mkdocs gh-deploy
```
