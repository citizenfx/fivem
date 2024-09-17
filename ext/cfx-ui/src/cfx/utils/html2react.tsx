import { identity } from '@cfx-dev/ui-components';
import * as DomHandler from 'domhandler';
import HRP, { Element, HTMLReactParserOptions } from 'html-react-parser';
import React from 'react';

const VERY_FAKE_DOMAIN = 'xn--80abujr4d.xn--90agu3en';

export interface Ihtml2reactOptions {
  removeRelativeLinks?: boolean;
}

// eslint-disable-next-line @typescript-eslint/no-unused-vars
let PARSING_OPTIONS: Ihtml2reactOptions = {};

const ATTRIBUTE_MAPPER = new Map(
  Object.entries({
    class: identity,
    title: identity,
    alt: identity,
    src(src: string, node: DomHandler.Element) {
      if (node.tagName === 'img') {
        return src;
      }

      return '';
    },
    href(href: string): string {
      try {
        const url = new URL(href, `https://${VERY_FAKE_DOMAIN}`);

        if (url.protocol !== 'http:' && url.protocol !== 'https:') {
          return '';
        }

        if (url.host === VERY_FAKE_DOMAIN) {
          return '';
        }

        return href;
      } catch (e) {
        return '';
      }
    },
    style(style: string) {
      return style.replace('url', '');
    },
  }),
);

const ALLOWED_TAGS = [
  'DIV',
  'SPAN',
  'CODE',
  'STRONG',
  'EM',
  'B',
  'A',
  'P',
  'UL',
  'LI',
  'OL',
  'IMG',
  'BR',
  'H1',
  'H2',
  'H3',
  'H4',
  'H5',
  'H6',
  'DETAILS',
];

const HRPOptions: HTMLReactParserOptions = {
  replace(node) {
    if (!(node instanceof Element)) {
      return;
    }

    if (DomHandler.isTag(node)) {
      const uctagName = node.tagName.toUpperCase();

      if (!ALLOWED_TAGS.includes(uctagName)) {
        return (
          // eslint-disable-next-line react/jsx-no-useless-fragment
          <></>
        );
      }
    }

    if (node.attribs) {
      const newAttribs = {};

      for (const [attrName, attrMapper] of ATTRIBUTE_MAPPER) {
        const attrValue = node.attribs[attrName];

        if (!attrValue) {
          continue;
        }

        const mappedAttrValue = attrMapper(attrValue, node);

        if (mappedAttrValue) {
          newAttribs[attrName] = mappedAttrValue;
        }
      }

      node.attribs = newAttribs;
    }

    return node;
  },
};

export function html2react(html: string, options: Ihtml2reactOptions = {}): React.ReactNode {
  PARSING_OPTIONS = options;

  return HRP(html, HRPOptions);
}
(globalThis as any).__html2react = html2react;
