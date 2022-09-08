import React from 'react';
import format from 'date-fns/format';
import formatDistanceToNow from 'date-fns/formatDistanceToNow';
import { Flex } from 'cfx/ui/Layout/Flex/Flex';
import { IActivityItem, IActivityItemMedia } from 'cfx/common/services/activity/types';
import { Title } from '../Title/Title';
import { Text } from '../Text/Text';
import { Avatar } from '../Avatar/Avatar';
import { Box } from '../Layout/Box/Box';
import { useBlurhash } from 'cfx/utils/useBlurhash';
import s from './ActivityItem.module.scss';
import { $L } from 'cfx/common/services/intl/l10n';

export interface ActivityItemProps {
  item: IActivityItem,
}

export function ActivityItem(props: ActivityItemProps) {
  const { item } = props;

  // #TODOLOC
  const formattedDate = format(item.date, 'MM/dd/yyyy @ h:mma');

  const media = item.media[0];

  return (
    <Flex gap="small">
      <Avatar size="small" url={item.userAvatarUrl} />

      <Box grow>
        <Flex vertical gap="small">
          <Title delay={200} fixedOn="bottom-left" title={item.userScreenName}>
            <Text weight='bold' opacity="75">
              {item.userDisplayName}
            </Text>
          </Title>

          <Title delay={200} fixedOn='bottom-left' title={<>{formattedDate}<br />{$L('#Feed_OpenInBrowser')}</>}>
            <Text opacity="50">
              <a href={item.url}>
                {formatDistanceToNow(item.date, { addSuffix: true })}
              </a>
            </Text>
          </Title>

          <div className={s.content}>
            {item.content}
          </div>

          {media && (
            <Media media={media} />
          )}
        </Flex>
      </Box>
    </Flex>
  );
}

function Media({ media }: { media: IActivityItemMedia }) {
  const [hovered, setHovered] = React.useState(false);

  const handleMouseOver = React.useCallback(() => setHovered(true), []);
  const handleMouseLeave = React.useCallback(() => setHovered(false), []);

  let view: React.ReactNode = null;

  switch (media.type) {
    case 'animated_gif':
    case 'photo': {
      view = (
        <img
          src={media.previewUrl}
          loading="lazy"
        />
      );
      break;
    }

    case 'video': {
      view = (
        <VideoPreview
          url={media.fullUrl}
          hovered={hovered}
        />
      );
      break;
    }
  }

  const previewURL = getPreviewURL(media);

  const style: any = {
    '--aspect-ratio': media.previewAspectRatio,
    backgroundImage: previewURL
      ? `url(${previewURL})`
      : '',
  };

  return (
    <div
      style={style}
      className={s.media}
      onMouseOver={handleMouseOver}
      onMouseLeave={handleMouseLeave}
    >
      {view}
    </div>
  );
}

function getPreviewURL(media: IActivityItemMedia): string {
  if (media.blurhash) {
    return useBlurhash(media.blurhash, 128, 128) || '';
  }

  if (media.type === 'photo') {
    return '';
  }

  return media.previewUrl || '';
}

function VideoPreview({ url, hovered }: { url: string, hovered: boolean }) {
  const viewRef = React.useRef<HTMLVideoElement>(null);

  React.useEffect(() => {
    const video = viewRef.current;

    if (video instanceof HTMLVideoElement) {
      if (video.readyState < video.HAVE_CURRENT_DATA) {
        return;
      }

      if (hovered && video.paused) {
        video.play();
      } else {
        video.pause();
      }
    }
  }, [hovered]);

  return (
    <video
      ref={viewRef}
      muted
      loop
      src={url}
    />
  );
}
