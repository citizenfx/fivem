import {
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
import s from './ChangelogPage.module.scss';

type SelectOptions = React.ComponentProps<typeof Select>['options'];

export const ChangelogPage = observer(function ChangelogPage() {
  const ChangelogService = useService(IChangelogService);
  const scrollableRef = React.useRef<HTMLDivElement>(null);
  const versionRefs = React.useRef<Record<string, HTMLDivElement | null>>({});
  
  // Zeroing unseen versions counter
  React.useEffect(() => {
    ChangelogService.maybeMarkNewAsSeen();
  }, []);

  const { versions } = ChangelogService;

  const versionItemsSelect: SelectOptions = React.useMemo(
    () => versions.map((version) => ({
      value: version,
      label: version,
    })),
    [versions],
  );
  
  React.useEffect(() => {
    if (ChangelogService.selectedVersion && versionRefs.current[ChangelogService.selectedVersion]) {
      const element = versionRefs.current[ChangelogService.selectedVersion];
      if (element && scrollableRef.current) {
        setTimeout(() => {
          element.scrollIntoView({ behavior: 'smooth', block: 'start' });
        }, 100);
      }
    }
  }, [ChangelogService.selectedVersion]);
  
  const generateDistinctiveColor = (str: string, index: number): string => {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      hash = str.charCodeAt(i) + ((hash << 5) - hash);
    }
    
    const ColorRatio = 0.618033988749895;
    let h = ((hash % 360) / 360 + index * ColorRatio) % 1.0;
    
    h = Math.floor(h * 360);
    
    const s = 65 + (index % 3) * 10;
    const l = 40 + (index % 4) * 5;
    
    return `hsl(${h}, ${s}%, ${l}%)`;
  };

  React.useEffect(() => {
    if (!versions.length) return;
    
    const observerOptions = {
      root: scrollableRef.current,
      rootMargin: '100px 0px 100px 0px',
      threshold: 0.1
    };
    
    const observerCallback = (entries: IntersectionObserverEntry[], observer: IntersectionObserver) => {
      entries.forEach(entry => {
        if (entry.isIntersecting) {
          const element = entry.target as HTMLElement;
          const version = element.dataset.version;
          if (version) {
            ChangelogService.getVersionContent(version);
            observer.unobserve(element);
          }
        }
      });
    };
    
    const observer = new IntersectionObserver(observerCallback, observerOptions);
    
    Object.values(versionRefs.current).forEach((ref: HTMLElement | null) => {
      if (ref) observer.observe(ref);
    });
    
    return () => observer.disconnect();
  }, [versions.length]);

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
        <Scrollable ref={scrollableRef}>
          <Pad size="large">
            <Prose>
              {ChangelogService.hasAnyVersions ? (
                versions.slice(0, 100).map((version, index) => {
                  const color = generateDistinctiveColor(version, index);
                  const versionContent = ChangelogService.getVersionContent(version);
                  
                  return (
                    <div 
                      key={version} 
                      className={s.versionContainer}
                      ref={el => {
                        versionRefs.current[version] = el;
                      }}
                      data-version={version}
                      id={`changelog-version-${version}`}
                    >
                      {/* version indicator with date */}
                      <div className={s.headerContainer}>
                        <div 
                          className={s.buildBadge}
                          onClick={() => ChangelogService.selectVersion(version)}
                        >
                          <div 
                            className={s.buildNumber}
                            style={{ backgroundColor: color }}
                          >
                            Build {version}
                          </div>
                          
                          {ChangelogService.getBuildDate(version) && (
                            <div className={s.buildDate}>
                              <svg 
                                xmlns="http://www.w3.org/2000/svg" 
                                width="14" 
                                height="14" 
                                viewBox="0 0 24 24" 
                                fill="none" 
                                stroke="currentColor" 
                                strokeWidth="2" 
                                strokeLinecap="round" 
                                strokeLinejoin="round" 
                                className={s.dateIcon}
                              >
                                <rect x="3" y="4" width="18" height="18" rx="2" ry="2"></rect>
                                <line x1="16" y1="2" x2="16" y2="6"></line>
                                <line x1="8" y1="2" x2="8" y2="6"></line>
                                <line x1="3" y1="10" x2="21" y2="10"></line>
                              </svg>
                              {ChangelogService.getBuildDate(version)}
                            </div>
                          )}
                        </div>
                        <div 
                          className={s.divider}
                          style={{ backgroundColor: color }}
                        />
                      </div>
                      
                      {/* changelog content */}
                      <div>
                        {versionContent !== undefined ? (
                          versionContent
                        ) : (
                          <div className={s.loadingContainer}>
                            <Text size="small" opacity="75">
                              <span className={s.loadingText}>Loading changelog...</span>
                            </Text>
                            <div 
                              className={s.loadingSpinner} 
                              style={{ 
                                borderColor: `${color} transparent ${color} transparent` 
                              }} 
                            />
                          </div>
                        )}
                      </div>
                    </div>
                  );
                })
              ) : (
                <div>
                  <span>🐌 Nothing to display right now check back later!</span>
                </div>
              )}
            </Prose>
          </Pad>
        </Scrollable>
      </Island>
    </Page>
  );
});
