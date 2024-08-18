import { Title, matchLinks } from '@cfx-dev/ui-components';
import { decode } from 'html-entities';
import React from 'react';

import { nl2brx } from 'cfx/utils/nl2br';

import { IActivityItem, IActivityItemData } from './types';

type LinksType = ReturnType<typeof matchLinks>;

export function compileActivityItem(data: IActivityItemData): IActivityItem {
  return {
    id: data.id,
    content: compileText(data.content, data.links),
    media: data.media,
    url: data.url,
    date: data.date,
    userAvatarUrl: data.userAvatarUrl,
    userDisplayName: data.userDisplayName,
    userScreenName: data.userScreenName,
    repostedBy: data.repostedBy,
  };
}

export function compileText(text: Map<number, string>, links: LinksType): React.ReactNode {
  const linkItems = Object.fromEntries(links.map((link) => [link.indices[0], link.url]));

  const parts: React.ReactNode[] = [];

  for (const [index, textItem] of text.entries()) {
    if (!textItem.trim()) {
      continue;
    }

    const key = index + textItem;

    if (linkItems[index]) {
      parts.push(
        <Title key={key} title="Opens in browser">
          <a href={textItem}>{linkItems[index]}</a>
        </Title>,
      );
    } else {
      parts.push(<React.Fragment key={key}>{nl2brx(decode(textItem))}</React.Fragment>);
    }
  }

  return parts;
}
