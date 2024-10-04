import { linkify, noop } from '@cfx-dev/ui-components';
import React from 'react';

import { CurrentGameBrand } from 'cfx/base/gameRuntime';
import { IServerView } from 'cfx/common/services/servers/types';
import { html2react } from 'cfx/utils/html2react';
import { invariant } from 'cfx/utils/invariant';
import { renderMarkdown } from 'cfx/utils/markdown';
import { nl2br } from 'cfx/utils/nl2br';
import { fastRandomId } from 'cfx/utils/random';

import { mpMenu } from '../mpMenu';
import { useStreamerMode } from '../services/convars/convars.service';

export interface IExtraAction {
  id: string;
  label: string;
  action(): void;
}

export interface IFormattedMessage {
  message: string;
  messageFormatted: boolean;

  title?: string;
  extraActions?: IExtraAction[];
}

export const FORMATTED_MESSAGE_MAGIC = '[md]';

export function isFormattedMessage(message: string): boolean {
  return message.startsWith(FORMATTED_MESSAGE_MAGIC);
}

export function maybeParseFormattedMessage(message: string): IFormattedMessage {
  if (isFormattedMessage(message)) {
    return parseFormattedMessage(message);
  }

  return {
    message,
    messageFormatted: false,
  };
}

export function parseFormattedMessage(message: string): IFormattedMessage {
  invariant(isFormattedMessage(message), 'Not a formatted message');

  const $html = document.createElement('div');
  $html.innerHTML = renderMarkdown(message.substring(FORMATTED_MESSAGE_MAGIC.length));

  const fmt: IFormattedMessage = {
    message: '', // Can't serialize yet
    messageFormatted: true,
  };

  const $aNodes = $html.querySelectorAll('a[href^="cfx.re://"');

  if ($aNodes.length) {
    fmt.extraActions = [];

    for (const $aNode of $aNodes) {
      fmt.extraActions.push({
        id: fastRandomId(),
        label: $aNode.innerHTML,
        action: getCfxReAnchorAction($aNode.getAttribute('href')!),
      });

      $aNode.parentNode?.removeChild($aNode);
    }
  }

  const $h1Node = $html.querySelector('h1');

  if ($h1Node) {
    fmt.title = $h1Node.innerText;

    $h1Node.parentNode?.removeChild($h1Node);
  }

  fmt.message = $html.innerHTML;

  return fmt;
}

export function useRenderedFormattedMessage<T extends IFormattedMessage>(
  msg: T,
  server?: IServerView,
): React.ReactNode {
  const streamerMode = useStreamerMode();

  return React.useMemo(() => {
    if (msg.messageFormatted) {
      return html2react(msg.message);
    }

    return html2react(
      linkify(nl2br(replaceCfxRePlaceholders(streamerMode
        ? censorServerAddress(msg.message)
        : msg.message, server))),
    );
  }, [msg, streamerMode]);
}

namespace CfxRePlaceholders {
  export const STATUSPAGE_STR = '[STATUSPAGE]';
  export const STATUSPAGE_RE = /\[STATUSPAGE\]/g;
  export const SUPPORT_RE = /\[SUPPORT\]/g;

  export const OWNER_STR = '[OWNER]';
  export const OWNER_RE = /\[OWNER\]/g;

  export const REJECTION_STR = 'Connection rejected by server:';
}
export function replaceCfxRePlaceholders(message: string, server?: IServerView | null | undefined): string {
  if (message.includes(CfxRePlaceholders.STATUSPAGE_STR)) {
    return message
      .replace(CfxRePlaceholders.STATUSPAGE_RE, '<a href="https://status.cfx.re">Cfx.re Status</a>')
      .replace(CfxRePlaceholders.SUPPORT_RE, '<a href="https://aka.cfx.re/support">Cfx.re Support</a>');
  }

  if (server?.ownerName) {
    const name = server.ownerName;
    const avatar = server.ownerAvatar;

    const avatarNode = avatar
      ? `<img class="__inline_avatar" src="${avatar}" />`
      : '';
    const avatarLink = `<a href="https://forum.cfx.re/u/${name}">${avatarNode}${name}</a>`;

    const messageIsARejection = message.startsWith(CfxRePlaceholders.REJECTION_STR);
    const messageIsNotFromCfxRe = !message.includes('CitizenFX ticket was specified');

    if (messageIsARejection && messageIsNotFromCfxRe) {
      return [
        message.replace(CfxRePlaceholders.REJECTION_STR, `Connection rejected by ${avatarLink}'s server:`),
        '',
        '',
        `<strong>This is <em>not</em> a message from ${CurrentGameBrand}.</strong>`,
      ].join('\n');
    }

    if (message.includes(CfxRePlaceholders.OWNER_STR)) {
      return message.replace(CfxRePlaceholders.OWNER_RE, avatarLink);
    }
  } else if (message.includes(CfxRePlaceholders.OWNER_STR)) {
    // #TODO: localization!
    return message.replace(CfxRePlaceholders.OWNER_RE, 'the server owner');
  }

  return message;
}

// Matches `<ip>:<port>` and `<ip> port <port>`
const ipRegex = Array(4).fill('(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)').join('\\.');
const serverAddressRegex = new RegExp(String.raw`\b${ipRegex}(\s+port\s+|:)\d+\b`, 'g');

export function censorServerAddress(message: string): string {
  return message.replace(serverAddressRegex, '&lt;HIDDEN&gt;');
}

function getCfxReAnchorAction(href: string): () => void {
  if (href === 'cfx.re://reconnect') {
    return () => mpMenu.invokeNative('reconnect');
  }

  return noop;
}
