from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import json
import os

app = Flask(__name__, static_folder='static', static_url_path='')
CORS(app)

COMPILER_EXE = "compiler.exe"  # Windows; on Linux use "./compiler"


@app.route('/api/tokenize', methods=['POST'])
def tokenize():
    """Lexical analysis only – returns tokens from compiler.exe"""
    try:
        data = request.json
        source_code = data.get('sourceCode', '')

        if not source_code:
            return jsonify({'success': False, 'error': 'No source code provided'}), 400

        result = subprocess.run(
            [COMPILER_EXE, 'tokenize', source_code],
            capture_output=True,
            text=True,
            timeout=5
        )

        if result.returncode != 0:
            return jsonify({'success': False, 'error': result.stderr}), 500

        output = json.loads(result.stdout)
        return jsonify(output)

    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/compile', methods=['POST'])
def compile_code():
    """Full pipeline: lexical + parse tree + semantic + TAC + optimization"""
    try:
        data = request.json
        source_code = data.get('sourceCode', '')

        if not source_code:
            return jsonify({'success': False, 'error': 'No source code provided'}), 400

        result = subprocess.run(
            [COMPILER_EXE, 'compile', source_code],
            capture_output=True,
            text=True,
            timeout=10
        )

        if result.returncode != 0:
            return jsonify({'success': False, 'error': result.stderr}), 500

        output = json.loads(result.stdout)
        return jsonify(output)

    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/generate', methods=['POST'])
def generate_parser():
    """
    Dummy LL(1) grammar analyzer so your existing FIRST/FOLLOW/table UI works.
    You can later replace this with real C++ LL(1) output.
    """
    try:
        data = request.json
        grammar_text = data.get('grammar', '')

        if not grammar_text:
            return jsonify({'success': False, 'error': 'No grammar provided'}), 400

        return jsonify({
            'success': True,
            'data': {
                'startSymbol': 'E',
                'nonTerminals': ['E', "E'", 'T', "T'", 'F'],
                'terminals': ['id', '+', '*', '(', ')', '$'],
                'grammar': {
                    'E': ["T E'"],
                    "E'": ["+ T E'", 'ε'],
                    'T': ["F T'"],
                    "T'": ["* F T'", 'ε'],
                    'F': ['( E )', 'id']
                },
                'firstSets': {
                    'E': ['(', 'id'],
                    "E'": ['+', 'ε'],
                    'T': ['(', 'id'],
                    "T'": ['*', 'ε'],
                    'F': ['(', 'id']
                },
                'followSets': {
                    'E': [')', '$'],
                    "E'": [')', '$'],
                    'T': ['+', ')', '$'],
                    "T'": ['+', ')', '$'],
                    'F': ['*', '+', ')', '$']
                },
                'parsingTable': {
                    'E': {'id': "T E'", '(': "T E'"},
                    "E'": {'+': "+ T E'", ')': 'ε', '$': 'ε'},
                    'T': {'id': "F T'", '(': "F T'"},
                    "T'": {'+': 'ε', '*': "* F T'", ')': 'ε', '$': 'ε'},
                    'F': {'id': 'id', '(': '( E )'}
                }
            }
        })

    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/')
def serve_index():
    """Serve the main HTML page."""
    return send_from_directory(app.static_folder, 'index.html')


if __name__ == '__main__':
    if not os.path.exists(COMPILER_EXE):
        print(f"WARNING: {COMPILER_EXE} not found!")
        print("Please compile compiler.cpp first: g++ compiler.cpp -o compiler.exe")

    print("Starting Flask server on http://localhost:5000")
    app.run(debug=True, port=5000)
