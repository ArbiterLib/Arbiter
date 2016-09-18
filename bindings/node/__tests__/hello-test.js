import hello from '../lib/hello.js';

test('hello', () => {
  expect(hello()).toBe('world');
});
