import * as React from 'react';
import classnames from 'classnames';
import AnsiToHTMLConverter from 'ansi-to-html';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import { observer } from 'mobx-react-lite';
import { OutputState } from 'store/OutputState';
import s from './Output.module.scss';

const converter = new AnsiToHTMLConverter({
  newline: true,
  colors: {
    0: '#5F5F5F',
    1: '#D96468',
    2: '#A2D964',
    3: '#D9C964',
    4: '#64A2D9',
    5: '#9A64D9',
    6: '#64D9D5',
    7: '#989898',
    8: '#828282',
    9: '#D98F93',
    10: '#B8D98F',
    11: '#D9CF8F',
    12: '#8F99D9',
    13: '#B08FD9',
    14: '#8FD9D5',
    15: '#C5C5C5',
  }
});

export interface OutputProps {
  channelId: string,

  className?: string,
}

export const Output = observer(function Output({ channelId, className }: OutputProps) {
  const output = OutputState.getOutput(channelId);
  const htmlOutput = React.useMemo(() => converter.toHtml(output), [output]);

  return (
    <ScrollContainer
      scrollToEnd
      className={classnames(s.root, className)}
    >
      <div
        dangerouslySetInnerHTML={{ __html: htmlOutput }}
      />
    </ScrollContainer>
  );
});
