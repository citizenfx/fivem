import markdownIt from 'markdown-it';

const md = markdownIt({
  breaks: true,
});

export function renderMarkdown(markdown: string): string {
  return md.render(markdown);
}
