import * as React from 'react';
import classnames from 'classnames';
import AnsiToHTMLConverter from 'ansi-to-html';
import { OutputContext } from 'contexts/OutputContext';
import s from './Output.module.scss';
import { useOpenFlag } from 'utils/hooks';

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

export const Output = React.memo(function Output({ channelId, className }: OutputProps) {
  const { outputs } = React.useContext(OutputContext);
  const output = outputs[channelId] || '';

  const [override, setOverriden, releaseOverride] = useOpenFlag(false);
  const ref = React.useRef<HTMLDivElement>();

  React.useLayoutEffect(() => {
    const div = ref.current;

    if (!div || override) {
      return;
    }

    div.scrollTop = div.scrollHeight;
  }, [output, override]);

  const handleWheel = React.useCallback(() => {
    const div = ref.current;

    if (!div) {
      return;
    }

    if (div.scrollTop < div.scrollHeight - div.offsetHeight) {
      return setOverriden();
    }

    return releaseOverride();
  }, [setOverriden, releaseOverride]);

  const htmlOutput = React.useMemo(() => converter.toHtml(output), [output]);

  return (
    <div
      ref={ref}
      onWheel={handleWheel}
      className={classnames(s.root, className)}
      dangerouslySetInnerHTML={{ __html: htmlOutput }}
    />
  );
});
