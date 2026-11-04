"""Verify both the minimum edit count and replay of the printed edit script."""

import itertools
from pathlib import Path
import random
import subprocess
import sys
import unittest

BINARY = Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else Path('build/diff').resolve()


def minimum_edits(source, target):
    # Independent full-table LCS oracle; substitutions require delete + insert.
    common = [[0] * (len(target) + 1) for _ in range(len(source) + 1)]
    for i, left in enumerate(source, 1):
        for j, right in enumerate(target, 1):
            common[i][j] = (common[i - 1][j - 1] + 1 if left == right else
                            max(common[i - 1][j], common[i][j - 1]))
    return len(source) + len(target) - 2 * common[-1][-1]


def replay(source, script):
    result = bytearray()
    position = edits = index = 0
    escapes = {ord('n'): b'\n', ord('r'): b'\r', ord('t'): b'\t',
               ord('\\'): b'\\', ord('{'): b'{', ord('}'): b'}'}

    def character(offset):
        if script[offset] == ord('\\'):
            return escapes[script[offset + 1]], offset + 2
        return script[offset:offset + 1], offset + 1

    while index < len(script):
        if script[index] != ord('{'):
            value, index = character(index)
            if source[position:position + 1] != value:
                raise AssertionError('Unchanged character does not match source')
            result.extend(value)
            position += 1
            continue
        operation = script[index + 1]
        if operation not in (ord('+'), ord('-')):
            raise AssertionError('Unknown operation')
        index += 2
        chunk = bytearray()
        while index < len(script) and script[index] != ord('}'):
            value, index = character(index)
            chunk.extend(value)
        if index == len(script) or not chunk:
            raise AssertionError('Invalid edit group')
        index += 1
        edits += len(chunk)
        if operation == ord('+'):
            result.extend(chunk)
        else:
            if source[position:position + len(chunk)] != chunk:
                raise AssertionError('Deleted text does not match source')
            position += len(chunk)
    if position != len(source):
        raise AssertionError('Source suffix was not consumed')
    return bytes(result), edits


class DiffTests(unittest.TestCase):
    def run_diff(self, source, target, stdin=None):
        args = [] if stdin is not None else [source, target]
        result = subprocess.run([str(BINARY), *args], input=stdin,
                                capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, b'')
        distance, script = result.stdout.replace(b'\r\n', b'\n').split(b'\n', 1)
        self.assertTrue(script.endswith(b'\n'))
        script = script[:-1]
        self.assertNotIn(b'\n', script)
        actual, edits = replay(source.encode(), script)
        self.assertEqual(actual, target.encode(), (source, target, script))
        self.assertEqual(edits, int(distance), (source, target, script))
        self.assertEqual(int(distance), minimum_edits(source.encode(), target.encode()))
        return script

    def test_exhaustive_small_strings(self):
        words = [''.join(chars) for length in range(5)
                 for chars in itertools.product('ab', repeat=length)]
        for source, target in itertools.product(words, repeat=2):
            with self.subTest(source=source, target=target):
                self.run_diff(source, target)

    def test_trailing_edits_are_printed(self):
        self.assertEqual(self.run_diff('abc', 'ab'), b'ab{-c}')
        self.assertEqual(self.run_diff('ab', 'abc'), b'ab{+c}')
        self.assertEqual(self.run_diff('abc', 'abd'), b'ab{-c}{+d}')
        self.assertEqual(self.run_diff('abc', 'xyz'), b'{-abc}{+xyz}')

    def test_spaces_and_literal_edit_markers(self):
        for source, target in [('a {+b} \\', 'a {-b} \\'), ('hello world', 'hello there'),
                               ('{', '}'), ('\\n', '\n'), ('a\tb\rc\n', 'a b c'), ('+-{}', '')]:
            with self.subTest(source=source, target=target):
                self.run_diff(source, target)

    def test_randomized_scripts(self):
        rng = random.Random(1)
        alphabet = 'abCD +-=}{\\\t'
        for _ in range(100):
            source = ''.join(rng.choices(alphabet, k=rng.randrange(50)))
            target = ''.join(rng.choices(alphabet, k=rng.randrange(50)))
            self.run_diff(source, target)

    def test_repetitive_and_unbalanced_inputs(self):
        for source, target in [('a' * 200 + 'b', 'b' + 'a' * 200),
                               ('a' * 4096, 'a'), ('a', 'a' * 4096), ('ab' * 128, 'ba' * 128)]:
            self.run_diff(source, target)

    def test_original_stdin_format_and_empty_strings(self):
        for source, target, data in [('abc', 'abd', b'3 abc 3 abd\n'),
                                     ('', 'abc', b'0 3 abc\n'),
                                     ('abc', '', b'3 abc 0\n'), ('', '', b'0 0\n'),
                                     ('abc', 'ab', b'3\nabc\n2\nab\n')]:
            self.run_diff(source, target, stdin=data)

    def test_invalid_input_fails_without_partial_output(self):
        for data in (b'', b'no input', b'-1 abc 3 abc', b'4 abc 3 abc',
                     b'3 abc', b'0 -1', b'999999999999999999999999 abc 0'):
            with self.subTest(data=data):
                result = subprocess.run([str(BINARY)], input=data, capture_output=True, timeout=5)
                self.assertEqual(result.returncode, 2)
                self.assertEqual(result.stdout, b'')
                self.assertTrue(result.stderr)

    def test_help_and_missing_argument(self):
        result = subprocess.run([str(BINARY), '--help'], capture_output=True, timeout=5)
        self.assertEqual(result.returncode, 0)
        self.assertIn(b'Usage:', result.stdout)
        result = subprocess.run([str(BINARY), 'abc'], capture_output=True, timeout=5)
        self.assertEqual(result.returncode, 2)
        self.assertIn(b'Usage:', result.stderr)


if __name__ == '__main__':
    unittest.main()
