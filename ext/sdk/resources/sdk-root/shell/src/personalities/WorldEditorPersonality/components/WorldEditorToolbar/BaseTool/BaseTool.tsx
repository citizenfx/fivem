import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { getToolCommand, getToolSide, WETool, WEToolbarState } from '../../../store/WEToolbarState';
import s from './BaseTool.module.scss';
import { Title } from 'components/controls/Title/Title';
import { WEHotkeysState } from 'personalities/WorldEditorPersonality/store/WEHotkeysState';

export interface BaseToolProps {
  tool: WETool,
  icon: React.ReactNode,
  label: string,

  highlight?: boolean,
  renderAlways?: boolean,
  children?: React.ReactNode,

  toggleClassName?: string,
  panelClassName?: string,

  toggleProps?: null | object,
  panelProps?: null | object,
}

export const BaseTool = observer(function BaseTool(props: BaseToolProps) {
  const {
    tool,
    icon,
    label,
    children,
    highlight = false,
    renderAlways = false,
    toggleClassName: tClassName = '',
    panelClassName: pClassName = '',

    toggleProps = null,
    panelProps = null,
  } = props;

  const toolIsOpen = WEToolbarState.isToolOpen(tool);

  const toggleClassName = classnames(s.toggle, s.hoverable, tClassName, {
    [s.active]: toolIsOpen,
    [s.highlight]: highlight,
  });

  const panelClassName = classnames(s.panel, pClassName, {
    [s.active]: toolIsOpen,
    [s.right]: getToolSide(tool) === 'right',
  });

  let childrenNode: React.ReactNode = null;
  if (children && (renderAlways || toolIsOpen)) {
    childrenNode = (
      <div className={panelClassName} {...panelProps}>
        {children}
      </div>
    );
  }

  const toolCommand = getToolCommand(tool);
  const shortcut = toolCommand
    ? WEHotkeysState.getCommandHotkey(toolCommand)
    : '';

  return (
    <>
      <Title animated={false} title={label} delay={0} fixedOn="top" shortcut={shortcut}>
        {(ref) => (
          <button
            {...toggleProps}
            ref={ref}
            className={toggleClassName}
            onClick={() => WEToolbarState.toggleTool(tool)}
          >
            {icon}
          </button>
        )}
      </Title>

      {childrenNode}
    </>
  );
});
