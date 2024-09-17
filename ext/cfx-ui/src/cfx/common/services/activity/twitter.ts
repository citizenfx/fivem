import { splitByIndices } from '@cfx-dev/ui-components';

import { IActivityItemData, IActivityItemMedia, IRawTweet, IRawTweetTypes } from './types';

function parseYoutube(url: string): [boolean, string] {
  const parsedUrl = new URL(url);

  if (parsedUrl.hostname === 'youtu.be' && parsedUrl.pathname.length >= 5) {
    return [true, parsedUrl.pathname.substring(1)];
  }

  if (parsedUrl.hostname === 'youtube.com' && parsedUrl.pathname.startsWith('/watch')) {
    const v = parsedUrl.searchParams.get('v');

    if (v) {
      return [true, v];
    }
  }

  return [false, ''];
}

function isYoutube(url: string) {
  const [valid] = parseYoutube(url);

  return valid;
}

export function rawTweetToActivityDataItem(rawTweet: IRawTweet): IActivityItemData | null {
  let actualTweet = rawTweet;
  let repostedBy: string | undefined;

  if (rawTweet.retweeted_status) {
    actualTweet = rawTweet.retweeted_status;
    repostedBy = actualTweet.user.name;
  }

  const fallbackContent = new Map([[0, actualTweet.text]]);
  let content: Map<number, string>;
  const media: IActivityItemMedia[] = [];

  try {
    if (actualTweet.extended_entities?.media) {
      for (const mediaEntity of actualTweet.extended_entities.media) {
        const previewUrl = mediaEntity.media_url_https || mediaEntity.media_url;

        const mediaItem: IActivityItemMedia = {
          id: mediaEntity.id_str,
          type: mediaEntity.type as any,

          previewAspectRatio: mediaEntity.sizes.large.w / mediaEntity.sizes.large.h,
          previewUrl,

          fullAspectRatio: 1,
          fullUrl: '',
        };

        mediaItem.fullAspectRatio = mediaEntity.sizes.large.w / mediaEntity.sizes.large.h;
        mediaItem.fullUrl = previewUrl;

        if (mediaEntity.type === 'animated_gif' || mediaEntity.type === 'video') {
          mediaItem.fullAspectRatio = mediaEntity.video_info.aspect_ratio[0] / mediaEntity.video_info.aspect_ratio[1];
          mediaItem.fullUrl = mediaEntity.video_info.variants
            .filter((a) => a.content_type?.startsWith('video/'))
            .sort((a, b) => b.bitrate - a.bitrate)[0].url;
        }

        media.push(mediaItem);
      }
    }

    const youtubeIndices: IRawTweetTypes.Indices[] = [];

    // some tweets (e.g. https://twitter.com/Lucas7yoshi_RS/status/1665445339946418178) may have both an embed and a
    // YT link
    if (media.length === 0) {
      const youtubeUrls = actualTweet.entities?.urls?.filter((x) => isYoutube(x.expanded_url)) || [];

      for (const youtube of youtubeUrls) {
        const [_, videoId] = parseYoutube(youtube.expanded_url);

        media.push({
          id: videoId,
          type: 'youtube',

          previewAspectRatio: 16 / 9,
          previewUrl: `https://img.youtube.com/vi/${videoId}/maxresdefault.jpg`,

          fullAspectRatio: 16 / 9,
          fullUrl: `https://www.youtube.com/embed/${videoId}?enablejsapi=1&autoplay=1`,
        });

        youtubeIndices.push(youtube.indices);
      }
    }

    const mediaIndicesFull = (actualTweet.entities.media || [])
      .map((tweetMedia) => tweetMedia.indices)
      .concat(youtubeIndices);
    const mediaIndices = mediaIndicesFull.map(([x]) => x);

    const urlIndicesFull = (actualTweet.entities.urls || []).map((url) => url.indices);

    content = splitByIndices(
      actualTweet.text,
      mediaIndicesFull.flat().concat(urlIndicesFull.flat()),
      true,
    );

    for (const index of mediaIndices) {
      content.delete(index);
    }
  } catch (e) {
    content = fallbackContent;
  }

  try {
    const links = (actualTweet.entities.urls || []).map(
      (link) => ({
        indices: link.indices,
        url: link.display_url,
      }) as IActivityItemData['links'][0],
    );

    const activity: IActivityItemData = {
      id: actualTweet.id_str,
      url: `https://twitter.com/FiveM/status/${actualTweet.id_str}`,
      date: new Date(actualTweet.created_at),
      content,
      media,
      userAvatarUrl: actualTweet.user.profile_image_url_https || actualTweet.user.profile_image_url,
      userDisplayName: actualTweet.user.name,
      userScreenName: `@${actualTweet.user.screen_name}`,
      links,
      repostedBy,
    };

    return activity;
  } catch (e) {
    console.warn('Failed to convert raw tweet to activity data item', e);

    return null;
  }
}
