#!/usr/bin/env python3
import json
import re
import os
import urllib.request
import sys

# Configuration
OUTPUT_FILE = "fang/blocked_content.json"
SOURCES = {
    "ads": [
        "https://easylist.to/easylist/easylist.txt",
        "https://raw.githubusercontent.com/uBlockOrigin/uAssets/master/filters/filters.txt"
    ],
    "privacy": [
        "https://easylist.to/easylist/easyprivacy.txt",
        "https://raw.githubusercontent.com/uBlockOrigin/uAssets/master/filters/privacy.txt"
    ],
    "annoyance": [
        "https://easylist.to/easylist/fanboy-annoyance.txt"
    ],
    "unbreak": [
        "https://raw.githubusercontent.com/uBlockOrigin/uAssets/master/filters/unbreak.txt"
    ]
}

def download_list(url):
    print(f"Downloading {url}...")
    try:
        with urllib.request.urlopen(url) as response:
            return response.read().decode('utf-8')
    except Exception as e:
        print(f"Error downloading {url}: {e}")
        return ""

def parse_adblock_rule(line):
    line = line.strip()
    if not line or line.startswith("!") or line.startswith("["):
        return None

    # Handle element hiding rules: example.com##.ad
    if "##" in line:
        parts = line.split("##", 1)
        domain_part = parts[0]
        selector = parts[1]
        
        trigger = {"url-filter": ".*"}
        if domain_part:
            domains = domain_part.split(",")
            valid_domains = []
            for d in domains:
                try:
                    # Lowercase and punycode
                    d = d.strip().lower().encode('idna').decode('ascii')
                    if d:
                        valid_domains.append(d)
                except:
                    pass
            
            if valid_domains:
                trigger["if-domain"] = valid_domains
        
        return {
            "trigger": trigger,
            "action": {
                "type": "css-display-none",
                "selector": selector
            }
        }

    # Handle basic blocking rules
    # ||example.com^
    if line.startswith("||"):
        pattern = line[2:]
        # Remove options for now to get the domain/pattern
        if "$" in pattern:
            pattern = pattern.split("$")[0]
        
        # Handle the ^ separator
        pattern = pattern.replace("^", "")
        
        # Simple approach: use the domain as if-domain if it looks like a domain
        if "/" not in pattern and "*" not in pattern:
             try:
                 # Ensure lowercase and punycode for domains
                 domain = pattern.lower().encode('idna').decode('ascii')
                 return {
                    "trigger": {
                        "url-filter": ".*",
                        "if-domain": [domain]
                    },
                    "action": {
                        "type": "block"
                    }
                }
             except:
                 return None
        
        # If it has path or wildcards, treat as regex
        # WebKit regex limitations: avoid complex constructs if possible
        # Escape dots
        regex = pattern.replace(".", "\\.")
        regex = regex.replace("*", ".*")
        
        # WebKit doesn't like some regex features. 
        # If it contains |, it might be problematic if not carefully constructed.
        # For now, let's skip rules with | to avoid "Disjunctions are not supported" if that's the issue.
        if "|" in regex:
            return None

        return {
            "trigger": {
                "url-filter": ".*" + regex + ".*"
            },
            "action": {
                "type": "block"
            }
        }

    return None

def main():
    # Ensure output directory exists
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

    for category, urls in SOURCES.items():
        category_rules = []
        print(f"Processing category: {category}...")
        
        for url in urls:
            content = download_list(url)
            lines = content.splitlines()
            print(f"Processing {len(lines)} lines from {url}...")
            
            count = 0
            for line in lines:
                rule = parse_adblock_rule(line)
                if rule:
                    category_rules.append(rule)
                    count += 1
            print(f"Added {count} rules to {category}.")
        
        output_filename = f"fang/blocked_content_{category}.json"
        with open(output_filename, "w") as f:
            json.dump(category_rules, f, indent=2)
        print(f"Saved {len(category_rules)} rules to {output_filename}")

if __name__ == "__main__":
    main()
