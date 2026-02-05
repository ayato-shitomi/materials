# セットアップ

```
git clone git@github.com:ayato-shitomi/materials.git
cd ./materials/202602_AI-driven-offensive-sec/waf_bypass

# Dockerを使う方
docker compose up --build

# Pythonで実行される方
python3 -m venv venv && source ./venv/bin/activate
pip install Flask
python3 chal.py
```
