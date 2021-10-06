import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { ToolbarResizer } from './ToolbarResizer';
import { ToolbarState } from 'store/ToolbarState';
import { ShellState } from 'store/ShellState';
import { ProjectState } from 'store/ProjectState';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import { StatusBar } from './StatusBar/StatusBar';
import { ToolbarMainMenu } from './ToolbarMainMenu';
import { ToolbarParticipants } from './toolbarExtensions';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';
import s from './Toolbar.module.scss';

export const Toolbar = observer(function Toolbar() {
  const viewsClassName = classnames(s.pane, {
    [s.active]: ShellState.isMainPersonality,
  });

  const titleViewNodes = renderViewRegistryItems(ToolbarParticipants.getAllVisibleTitleViews());
  const viewNodes = renderViewRegistryItems(ToolbarParticipants.getAllVisibleViews());

  return (
    <div
      style={ToolbarState.cssVariables}
      className={s.root}
      data-intro-id="toolbar"
    >
      <ToolbarResizer />

      <div className={s['tools-bar']}>
        <div className={s.controls}>
          <ToolbarMainMenu />

          <div className={s['project-name']} title={ProjectState.projectName}>
            <span>
              {ProjectState.projectName}
            </span>
          </div>

          {titleViewNodes}
        </div>

        <div className={viewsClassName}>
          <ScrollContainer>
            {viewNodes}
          </ScrollContainer>
        </div>

        <StatusBar />
      </div>
    </div>
  );
});
