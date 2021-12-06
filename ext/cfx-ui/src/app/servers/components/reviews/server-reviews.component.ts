import { HttpErrorResponse } from '@angular/common/http';
import { Component, Inject, Input, OnInit } from '@angular/core';
import { L10nLocale, L10N_LOCALE } from 'angular-l10n';
import { DiscourseService } from 'app/discourse.service';
import { GameService } from 'app/game.service';
import { Server } from 'app/servers/server';
import { filterProjectName } from 'app/servers/server-utils';
import { intervalToDuration } from 'date-fns';
import { Observable, of } from 'rxjs';
import { catchError, concatMap, map } from 'rxjs/operators';
import { PostCreateResponse, TagTopicsResponse, Topic, TopicPostsResponse, TopicResponse } from './discourse-models';

const formatDistanceLocale = { xSeconds: '{{count}} sec', xMinutes: '{{count}} min', xHours: '{{count}} h', xDays: '{{count}} d' };
const shortEnLocale: Locale = { formatDistance: (token, count) => formatDistanceLocale[token].replace('{{count}}', count) };

export interface TopicEntry {
	topic: Topic;
	detail: Observable<TopicPostsResponse>;
}

@Component({
	moduleId: module.id,
	selector: 'app-server-reviews',
	templateUrl: 'server-reviews.component.html',
	styleUrls: ['server-reviews.component.scss']
})
export class ServerReviewsComponent implements OnInit {
	@Input()
	server: Server;

	reviewTitle: string;
	reviewBody: string;
	reviewRecommended: null | 'yes' | 'no' = null;

	submitting = false;
	error = false;
	errors = '';

	shortEnLocale = shortEnLocale;

	private _reviews: TopicEntry[] = [];
	playtimes: Record<string, number> = {};

	constructor(private discourse: DiscourseService, @Inject(L10N_LOCALE) public locale: L10nLocale,
		private gameService: GameService) {

	}

	get reviews() {
		return this._reviews;
	}

	set reviews(reviews: TopicEntry[]) {
		this.fetchPlaytimes(reviews);

		this._reviews = reviews;
	}

	openSupport() {
		this.gameService.openUrl('https://support.cfx.re/');
	}

	duration(time?: number): Duration {
		return intervalToDuration({
			start: 0,
			end: (time ?? 0) * 1000
		});
	}

	async fetchPlaytimes(reviews: TopicEntry[]) {
		const identifiers = reviews
			.map(review => `fivem:${review.topic.posters[0].user_id}`)
			.filter(id => !(id in this.playtimes));

		if (this.myId) {
			identifiers.push(`fivem:${this.myId}`);
		}

		if (identifiers.length === 0) {
			return;
		}

		const searchParams = new URLSearchParams(identifiers.map(ident => ['identifiers[]', ident]));

		const r = await fetch(`https://lambda.fivem.net/api/ticket/playtimes/${this.server.address}?${searchParams}`);
		if (r.ok) {
			const data: any[] = await r.json();

			this.playtimes = {
				...this.playtimes,
				...(Object.fromEntries(data.map(e => [e.id, e.seconds])))
			};
		}
	}

	ngOnInit() {
		this.fetchReviews(0).subscribe(topics => this.reviews = topics);
	}

	get ownReviewId() {
		return this.reviews.find(a =>
			(a.topic.posters.find(b => b.description === 'Original Poster') ?? a.topic.posters[0]).user_id === this.myId)?.topic.id;
	}

	get canReview() {
		if (this.myId <= 0) {
			return false;
		}

		if (this.ownReviewId) {
			return false;
		}

		return true;
	}

	get myAvatar() {
		return this.discourse.currentUser?.getAvatarUrl();
	}

	get myUsername() {
		return this.discourse.currentUser?.username;
	}

	get myId() {
		return this.discourse.currentUser?.id ?? 0;
	}

	get myPlaytime() {
		return this.playtimes[this.myId] || 0;
	}

	get projectName() {
		return filterProjectName(this.server?.data?.vars?.sv_projectName || '');
	}

	private fetchReviews(page?: number) {
		const mapFn = (response: TagTopicsResponse) => {
			return response.topic_list.topics.map(topic => ({
				topic,
				detail: this.fetchTopicPosts(topic.id)
			}));
		}

		if (!this.myId) {
			return this.fetchAllReviews().pipe(map(mapFn));
		}

		// TODO: category id un-hardcode
		return this.fetchMyReviews()
			.pipe(
				concatMap(selfPosted => this.fetchAllReviews().pipe(map(all => ({
					...all,
					topic_list: {
						topics: [
							...selfPosted.topic_list.topics.slice(0, 1),
							...all.topic_list.topics.filter(a => !a.posters.find(b => b.user_id === this.myId))
						]
					}
				})))),
				map(mapFn));
	}

	private topicCatch() {
		return of<TagTopicsResponse>({
			topic_list: {
				can_create_topic: false,
				per_page: 0,
				top_tags: [],
				tags: [],
				topics: [],
			}
		});
	}

	private fetchMyReviews() {
		return this.discourse.apiCallObservable<TagTopicsResponse>(`/tags/c/76/${this.server.address}/l/posted.json`)
			.pipe(catchError(_ => this.topicCatch()));
	}

	private fetchAllReviews(page?: number) {
		return this.discourse.apiCallObservable<TagTopicsResponse>(`/tags/c/76/${this.server.address}.json?page=${page ?? 0}&ascending=false&order=likes`)
			.pipe(catchError(_ => this.topicCatch()));
	}

	private fetchTopicPosts(topicId: number) {
		return this.discourse.apiCallObservable<TopicPostsResponse>(`/t/${topicId}/posts.json`);
	}

	reviewTrack(_: number, row: TopicEntry) {
		return row.topic.id;
	}

	async submit() {
		this.submitting = true;
		this.error = false;

		try {
			const postResponse = await this.discourse.apiCallObservable<PostCreateResponse>('/posts', 'POST', {
				title: this.reviewTitle,
				raw: this.reviewBody,
				category: 76,
				tags: [
					this.server.address,
					this.reviewRecommended === 'yes' ? 'recommended' : 'unrecommended'
				],
				archetype: 'regular',
				typing_duration_msecs: 3000, // TODO
				composer_open_duration_msecs: 5000, // TODO?
				nested_post: true, // use the new api
			}).toPromise();

			if (!postResponse.success) {
				throw new Error(postResponse.errors.join('\n'));
			}

			if (postResponse.action === 'enqueued') {
				// TODO: flow to say the post is pending-review
				// TODO: mark my-review properly?
			} else {
				const topicResponse = await this.discourse.apiCallObservable<TopicResponse>(`/t/${postResponse.post.topic_id}`).toPromise();

				this.reviews.splice(0, 0, {
					topic: {
						...topicResponse,
						posters: [
							{
								user_id: this.myId,
								description: 'Original Poster',
							}
						],
					},
					detail: of(topicResponse),
				});

				this.reviews = this.reviews;
			}

			this.reviewBody = '';
			this.reviewTitle = '';
			this.reviewRecommended = null;
		} catch (e) {
			if (e instanceof HttpErrorResponse) {
				this.errors = e.error.errors.join('\n');
			} else {
				this.errors = e.message;
			}
			console.error(e);
			this.error = true;
		} finally {
			this.submitting = false;
		}
	}
}
