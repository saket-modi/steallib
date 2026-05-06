import re
import sys
from bs4 import BeautifulSoup

def clean_file(fn):
    with open(fn, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # regex search for <!DOCTYPE..> or <html> header
    match = re.search(r'<(?:!DOCTYPE|html)', content, re.IGNORECASE)
    
    if match:
        html_body = content[match.start():]
        
        # remove the chunked encoding terminator (0 at the end of the file)
        end_tag = re.search(r'</html>', html_body, re.IGNORECASE)
        if end_tag:
            html_body = html_body[:(end_tag.start() + 7)] # 7 for </html>
    else:
        print("no html content in file!")
        return

    # format html
    soup = BeautifulSoup(html_body, 'html.parser')
    
    # Save the cleaned and formatted version back to the file
    with open(fn, 'w', encoding='utf-8') as f:
        f.write(soup.prettify())

if __name__ == "__main__" and len(sys.argv) > 1:
    clean_file(sys.argv[1])