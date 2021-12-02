import * as React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { updaterStatuses } from 'shared/api.statuses';
import { AppStates } from 'shared/api.types';
import { BsArrowRepeat } from 'react-icons/bs';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { ShellState } from 'store/ShellState';
import { StatusState } from 'store/StatusState';
import s from './Updater.module.scss';
import { UpdaterViewParticipants } from './updaterExtensions';
import { UpdaterState } from './UpdaterState';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';

const defaultStatus = {
  completed: 0,
  currentFileName: '',
};

const preparingTitleNode = (
  <ul>
    <li>Preparing FxDK</li>
    <li>Unwrapping singularity</li>
    <li>Merging black holes</li>
    <li>Asserting dominance</li>
    <li>Fixing bugs</li>
    <li>Almost there</li>
    <li>Why are we here</li>
    <li>What is the point?</li>
    <li>Okay, this takes way too much time 😒</li>
    <li><BsArrowRepeat /></li>
  </ul>
);

const readyTitleNode = (
  <ul>
    <li>FxDK is ready!</li>
  </ul>
);

export const Updater = observer(function Update() {
  const status = StatusState.get(updaterStatuses.state, defaultStatus);

  const handleContinue = React.useCallback(() => {
    UpdaterState.close();
    UpdaterViewParticipants.handleClose();
  }, []);

  const isReady = ShellState.appState === AppStates.ready;
  const rootClassName = classnames(s.root, {
    [s.shrimp]: isReady,
  });

  const titleNode = isReady
    ? readyTitleNode
    : preparingTitleNode;

  const participantNodes = renderViewRegistryItems(UpdaterViewParticipants.getAllVisible());

  return (
    <div className={rootClassName}>
      <div className={s.content}>
        <h1 className={s.title}>
          {titleNode}
        </h1>

        <div className={s.progress}>
          <div className={s.bar} style={{ width: `${status.completed * 100}%` }}/>
        </div>
        <div className={s['current-file-name']}>
          {isReady ? 'Done!' : status.currentFileName}
        </div>

        {participantNodes}

        <div className={s.actions}>
          <Button
            theme="primary"
            text="Continue"
            onClick={handleContinue}
          />
        </div>
      </div>
    </div>
  );
});
