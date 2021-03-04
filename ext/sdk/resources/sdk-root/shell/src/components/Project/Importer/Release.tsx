import * as React from 'react';
import ReactMarkdown from 'react-markdown';
import classnames from 'classnames';
import s from './Release.module.scss';
import { ReleaseInfo } from 'shared/api.requests';

export interface ReleaseProps {
  info: ReleaseInfo,

  checked?: boolean,
  onClick?: () => void,
}

export const Release = React.memo(function Release(props: ReleaseProps) {
  const { info, checked, onClick } = props;

  if (!info.name) {
    return null;
  }

  const rootClassName = classnames(s.root, {
    [s.checked]: checked,
  });

  return (
    <div className={rootClassName} onClick={onClick}>
      <div className={s.content}>
        <div className={s.title}>
          {info.name}
        </div>
        {!!info.body && (
          <div className={s.description}>
            <ReactMarkdown children={info.body} />
          </div>
        )}
      </div>
    </div>
  );
});
