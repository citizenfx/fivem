import { Autolinker } from 'autolinker';
import React from 'react';
import { splitByIndices } from './string';

export function isExternalUrl(url: string): boolean {
  if (url.startsWith('http')) {
    return true;
  }

  if (url.startsWith('file://')) {
    return true;
  }

  return false;
}

export interface ILinkSubstitute {
  indices: [number, number],
  url: string,
}
export interface ILinkMatch {
  key: string,
  text: string,
  url: string,
}

const autolinker = new Autolinker();

export function matchLinks(text: string): ILinkSubstitute[] {
  const matches = autolinker.parse(text);

  return matches.map((match) => ({
    indices: [
      match.getOffset(),
      match.getOffset() + match.getMatchedText().length,
    ],
    url: match.getAnchorHref(),
  }));
}
export function* matchLinkNodes(text: string) {
  const linkSubstitutes = matchLinks(text);

  const indices: number[] = [];
  const replacers: Record<number, string> = {};

  for (const link of linkSubstitutes) {
    indices.push(...link.indices);
    replacers[link.indices[0]] = link.url;
  }

  for (const [index, str] of splitByIndices(text, indices)) {
    const url = replacers[index];

    if (url) {
      yield {
        key: index + str + url,
        text: str,
        url,
      };
    } else {
      yield str;
    }
  }
}
try {
  (window as any).__matchLinks = matchLinks;
} catch (e) {}

export function defaultLinkReplacerx(key: string, text: string, url: string): React.ReactNode {
  return (
    <a key={key} href={url}>
      {text}
    </a>
  );
}
export function defaultLinkReplacer(text: string, url: string): string {
  return `<a href="${url}">${text}</a>`;
}

export function linkifyx(text: string, replacer = defaultLinkReplacerx): React.ReactNode {
  const nodes: React.ReactNode[] = [];

  for (const link of matchLinkNodes(text)) {
    if (typeof link === 'string') {
      nodes.push(link);
      continue;
    }

    nodes.push(replacer(link.key, link.text, link.url));
  }

  return nodes;
}

export function linkify(text: string, replacer = defaultLinkReplacer): string {
  const nodes: string[] = [];

  for (const link of matchLinkNodes(text)) {
    if (typeof link === 'string') {
      nodes.push(link);
      continue;
    }

    nodes.push(replacer(link.text, link.url));
  }

  return nodes.join('');
}
try {
  (window as any).__linkifyx = linkifyx;
  (window as any).__linkify = linkify;
} catch (e) {}

export interface LinkifyProps {
  text: string,
  replacer?: typeof defaultLinkReplacerx,
}
export function Linkify(props: LinkifyProps) {
  const {
    text,
    replacer = defaultLinkReplacerx,
  } = props;

  const linkified = React.useMemo(() => linkifyx(text, replacer), [text, replacer]);

  return (
    <>
      {linkified}
    </>
  );
}
