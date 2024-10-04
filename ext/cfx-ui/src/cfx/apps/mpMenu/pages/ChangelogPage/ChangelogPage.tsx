import {
  Indicator,
  Island,
  Flex,
  Pad,
  Page,
  Scrollable,
  Prose,
  Select,
  Text,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { InsideNavBar } from 'cfx/apps/mpMenu/parts/NavBar/InsideNavBar';
import { useService } from 'cfx/base/servicesContainer';
import { $L } from 'cfx/common/services/intl/l10n';

import { IChangelogService } from '../../services/changelog/changelog.service';

type SelectOptions = React.ComponentProps<typeof Select>['options'];

export const ChangelogPage = observer(function ChangelogPage() {
  const ChangelogService = useService(IChangelogService);

  // Zeroing unseen versions counter
  React.useEffect(() => {
    ChangelogService.maybeMarkNewAsSeen();
  }, []);

  const {
    versions,
  } = ChangelogService;
  const versionItemsSelect: SelectOptions = React.useMemo(
    () => versions.map((version) => ({
      value: version,
      label: version,
    })),
    [versions],
  );

  return (
    <Page>
      <InsideNavBar>
        <Flex centered="axis">
          <Text size="large">{$L('#Changelogs')}</Text>

          <Select
            value={ChangelogService.selectedVersion}
            options={versionItemsSelect}
            onChange={ChangelogService.selectVersion}
          />
        </Flex>
      </InsideNavBar>

      <Island grow>
        <Scrollable>
          <Pad size="large">
            <Prose>
              {ChangelogService.selectedVersionContent || (
                <Indicator />
              )}
            </Prose>
          </Pad>
        </Scrollable>
      </Island>
    </Page>
  );
});
