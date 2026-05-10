import re
import sys
from bs4 import BeautifulSoup

def clean_file(fn, url):
    try:
        with open(fn, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"File {fn} not found.")
        return

    # skip headers and find start of html
    match = re.search(r'<(?:!DOCTYPE|html)', content, re.IGNORECASE)
    if not match:
        print("No HTML content detected (check if the request failed).")
        return
    
    html_body = content[match.start():]
    
    # remove chunked encoding garbage (the '0' at the end)
    end_match = list(re.finditer(r'</html>', html_body, re.IGNORECASE))
    if end_match:
        last_tag = end_match[-1]
        html_body = html_body[:last_tag.end()]

    # parse html
    soup = BeautifulSoup(html_body, 'html.parser')
    
    if soup.head:
        # create <base href="...">
        base_tag = soup.new_tag("base", href=url)
        # insert it at the beginning of the <head>
        soup.head.insert(0, base_tag)
    elif soup.html:
        # If there's no <head>, just put it inside <html>
        base_tag = soup.new_tag("base", href=url)
        soup.html.insert(0, base_tag)

    with open(fn, 'w', encoding='utf-8') as f:
        # prettify() for indentation
        f.write(soup.prettify())

if __name__ == "__main__":
    if len(sys.argv) > 2:
        clean_file(sys.argv[1], sys.argv[2])