import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import * as React from 'react';
import { changelogEntries, ChangelogEntry } from './Changelog.entries';
import s from './Changelog.module.scss';

const ChangelogNode = React.memo(function ChangelogNode(props: ChangelogEntry & { old?: boolean }) {
  return (
    <div className={s.entry} style={{ opacity: props.old ? .5 : 1 }}>
      <div className={s.title}>
        {props.title}
      </div>
      <div className={s.content}>
        {props.content}
      </div>
    </div>
  );
});

export const Changelog = React.memo(function Changelog() {
  const entriesNodes = changelogEntries.map((entry, index) => {
    if (index === 0) {
      return (
        <React.Fragment key={entry.id}>
          <ChangelogNode
            key={entry.id}
            {...entry}
          />

          <div className={s.older}>Older entries</div>
        </React.Fragment>
      );
    }

    return (
      <ChangelogNode
        old
        key={entry.id}
        {...entry}
      />
    );
  });

  return (
    <ScrollContainer className={s.root}>
      {entriesNodes}
    </ScrollContainer>
  );
});
