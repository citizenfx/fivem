import React from 'react';
import { observer } from 'mobx-react-lite';
import { Indicator } from 'components/Indicator/Indicator';
import { StatusCenter } from 'components/StatusCenter/StatusCenter';
import { BsListTask } from 'react-icons/bs';
import { TaskState } from 'store/TaskState';
import { WETool, WEToolbarState } from '../WEToolbarState';
import { BaseTool } from '../BaseTool/BaseTool';

const noop = () => {};

export const StatusTool = observer(function StatusTool() {
  const tasks = TaskState.values;

  const hasTasks = tasks.length > 0;

  const toolOpen = WEToolbarState.isToolOpen(WETool.StatusCenter);

  return (
    <BaseTool
      tool={WETool.StatusCenter}
      icon={hasTasks ? <Indicator /> : <BsListTask />}
      label="Notifications & tasks"
    >
      <StatusCenter
        open={toolOpen}
        onClose={noop}
      />
    </BaseTool>
  );
});
