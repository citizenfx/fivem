import * as React from 'react';
import { ResourceStatus } from 'assets/resource/resource-types';
import { Modal } from 'components/Modal/Modal';
import { Output } from 'components/Output/Output';
import { Button } from 'components/controls/Button/Button';
import { Select, SelectOption } from 'components/controls/Select/Select';
import { observer } from 'mobx-react-lite';
import { OutputState } from 'store/OutputState';
import s from './ResourceCommandsOutputModal.module.scss';

export interface ResourceCommandsOutputModalProps {
  onClose: () => void,
  resourceName: string,
  resourceStatus: ResourceStatus,
}

export const ResourceCommandsOutputModal = observer(function ResourceCommandsOutputModal(props: ResourceCommandsOutputModalProps) {
  const {
    onClose,
    resourceName,
    resourceStatus,
  } = props;

  const { watchCommands } = resourceStatus;

  const channelIds = React.useMemo(() => {
    return Object.values(watchCommands).map(({ outputChannelId }) => outputChannelId);
  }, [watchCommands]);

  const [channelId, setChannelId] = React.useState(channelIds[0] || '');

  const channelOptions: SelectOption<string>[] = React.useMemo(() => {
    return channelIds.map((cid) => ({
      value: cid,
      title: OutputState.getLabel(cid) || cid,
    }));
  }, [channelIds]);

  return (
    <Modal fullWidth onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Resource {resourceName} commands outputs
        </div>

        <div className="modal-label">
          Command
        </div>
        <Select
          value={channelId}
          onChange={setChannelId as any}
          options={channelOptions}
          className={s.select}
        />

        <Output
          channelId={channelId}
          className={s.output}
        />

        <div className="modal-actions">
          <Button
            text="Close"
            onClick={onClose}
          />
        </div>
      </div>
    </Modal>
  );
});
