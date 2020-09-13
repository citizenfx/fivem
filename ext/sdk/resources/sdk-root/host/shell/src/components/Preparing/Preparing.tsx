import React from 'react';
import classnames from 'classnames';
import { StateContext } from '../../contexts/StateContext';
import { States } from '../../sdkApi/api.types';
import s from './Preparing.module.scss';


interface DownloadProps {
  width: string,
  total: number,
  downloaded: number,
};
const Download = React.memo(({ width, total, downloaded }: DownloadProps) => {
  const barWidth = ((downloaded / total) * 100) + '%';

  return (
    <div className={s.download} style={{ width }}>
      <div className={s.progress}>
        <div
          className={s.bar}
          style={{ width: barWidth }}
        />
      </div>
    </div>
  );
});

const titles = {
  [States.booting]: 'FxDK is starting',
  [States.preparing]: 'FxDK is getting ready',
  [States.ready]: 'FxDK is ready!',
};

export const Preparing = React.memo(() => {
  const { state, fxserverDownload, fxserverUnpack } = React.useContext(StateContext);

  const isPreparing = state === States.preparing;

  const title = titles[state];
  const headerLeadClassName = classnames(s.lead, {
    [s.active]: isPreparing,
  });

  return (
    <div className={s.root}>
      <div className={s.header}>
        <h1>{title}</h1>
        <span className={headerLeadClassName}>
          Downloading magick wands and powders
        </span>
      </div>

      {isPreparing && (
        <div className={s.wrapper}>
          <Download
            width="50%"
            {...fxserverDownload}
          />
          <Download
            width="50%"
            {...fxserverUnpack}
          />
        </div>
      )}
    </div>
  );
});
