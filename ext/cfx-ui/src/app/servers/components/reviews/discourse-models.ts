export interface TagTopicsResponse {
    users?:          User[];
    primary_groups?: PrimaryGroup[];
    flair_groups?:   FlairGroup[];
    topic_list:      TopicList;
}

export interface FlairGroup {
    id:             number;
    name:           string;
    flair_url:      string;
    flair_bg_color: string;
    flair_color:    string;
}

export interface PrimaryGroup {
    id:   number;
    name: string;
}

export interface TopicList {
    can_create_topic: boolean;
    per_page:         number;
    top_tags:         string[];
    tags:             Tag[];
    topics:           Topic[];
}

export interface Tag {
    id:          number;
    name:        string;
    topic_count: number;
    staff:       boolean;
}

export interface Topic {
    id:                    number;
    title:                 string;
    fancy_title:           string;
    slug:                  string;
    posts_count:           number;
    reply_count:           number;
    highest_post_number:   number;
    image_url:             null;
    created_at:            Date;
    last_posted_at:        Date;
    bumped?:               boolean;
    bumped_at?:            Date;
    archetype:             string;
    unseen?:               boolean;
    last_read_post_number: number;
    unread?:               number;
    new_posts?:            number;
    unread_posts?:         number;
    pinned:                boolean;
    unpinned:              null;
    excerpt?:              string;
    visible:               boolean;
    closed:                boolean;
    archived:              boolean;
    notification_level?:   number;
    bookmarked:            boolean;
    liked?:                boolean;
    tags:                  string[];
    views:                 number;
    like_count:            number;
    has_summary:           boolean;
    last_poster_username?: string;
    category_id:           number;
    pinned_globally:       boolean;
    featured_link:         null;
    has_accepted_answer?:  boolean;
    can_have_answer?:      boolean;
    posters?:              Poster[];
}

export interface Poster {
    extras?:           string;
    description:       string;
    user_id:           number;
    primary_group_id?: number;
    flair_group_id?:   number;
}

export interface User {
    id:                 number;
    username:           string;
    avatar_template:    string;
    primary_group_name: string;
    flair_name:         string;
    flair_url:          string;
    flair_bg_color:     string;
    flair_color:        string;
    admin:              boolean;
    moderator:          boolean;
    trust_level:        number;
}

export interface TopicPostsResponse {
    post_stream:             PostStream;
    id:                      number;
}

export interface TopicResponse {
    post_stream:             PostStream;
    timeline_lookup:         Array<number[]>;
    suggested_topics:        SuggestedTopic[];
    tags:                    string[];
    id:                      number;
    title:                   string;
    fancy_title:             string;
    posts_count:             number;
    created_at:              Date;
    views:                   number;
    reply_count:             number;
    like_count:              number;
    last_posted_at:          Date;
    visible:                 boolean;
    closed:                  boolean;
    archived:                boolean;
    has_summary:             boolean;
    archetype:               string;
    slug:                    string;
    category_id:             number;
    word_count:              number;
    deleted_at:              null;
    user_id:                 number;
    featured_link:           null;
    pinned_globally:         boolean;
    pinned_at:               null;
    pinned_until:            null;
    image_url:               null;
    slow_mode_seconds:       number;
    draft:                   null;
    draft_key:               string;
    draft_sequence:          number;
    posted:                  boolean;
    unpinned:                null;
    pinned:                  boolean;
    current_post_number:     number;
    highest_post_number:     number;
    last_read_post_number:   number;
    last_read_post_id:       number;
    deleted_by:              null;
    has_deleted:             boolean;
    actions_summary:         TopicResponseActionsSummary[];
    chunk_size:              number;
    bookmarked:              boolean;
    bookmarked_posts:        null;
    topic_timer:             null;
    message_bus_last_id:     number;
    participant_count:       number;
    queued_posts_count:      number;
    show_read_indicator:     boolean;
    thumbnails:              null;
    slow_mode_enabled_until: null;
    valid_reactions:         string[];
    details:                 Details;
    pending_posts:           any[];
}

export interface TopicResponseActionsSummary {
    id:      number;
    count:   number;
    hidden:  boolean;
    can_act: boolean;
}

export interface Details {
    can_edit:                    boolean;
    notification_level:          number;
    notifications_reason_id:     number;
    can_move_posts:              boolean;
    can_delete:                  boolean;
    can_remove_allowed_users:    boolean;
    can_create_post:             boolean;
    can_reply_as_new_topic:      boolean;
    can_flag_topic:              boolean;
    can_convert_topic:           boolean;
    can_review_topic:            boolean;
    can_close_topic:             boolean;
    can_archive_topic:           boolean;
    can_split_merge_topic:       boolean;
    can_edit_staff_notes:        boolean;
    can_toggle_topic_visibility: boolean;
    can_pin_unpin_topic:         boolean;
    can_moderate_category:       boolean;
    can_remove_self_id:          number;
    participants:                Participant[];
    created_by:                  CreatedBy;
    last_poster:                 CreatedBy;
}

export interface CreatedBy {
    id:              number;
    username:        string;
    avatar_template: string;
}

export interface Participant {
    id:                 number;
    username:           string;
    avatar_template:    string;
    post_count:         number;
    primary_group_name: string;
    flair_name:         string;
    flair_url:          string;
    flair_color:        string;
    flair_bg_color:     string;
    admin:              boolean;
    moderator:          boolean;
    trust_level:        number;
}

export interface PostStream {
    posts:  Post[];
    stream: number[];
}

export interface CurrentUserReaction {
    id:       string;
    type:     string;
    can_undo: boolean;
}

export interface Reaction {
    id:    string;
    type:  string;
    count: number;
}

export interface Post {
    id:                              number;
    username:                        string;
    avatar_template:                 string;
    created_at:                      Date;
    cooked:                          string;
    post_number:                     number;
    post_type:                       number;
    updated_at:                      Date;
    reply_count:                     number;
    reply_to_post_number:            null;
    quote_count:                     number;
    incoming_link_count:             number;
    reads:                           number;
    readers_count:                   number;
    score:                           number;
    yours:                           boolean;
    topic_id:                        number;
    topic_slug:                      string;
    primary_group_name:              string;
    flair_name:                      string;
    flair_url:                       string;
    flair_bg_color:                  string;
    flair_color:                     string;
    version:                         number;
    can_edit:                        boolean;
    can_delete:                      boolean;
    can_recover:                     boolean;
    can_wiki:                        boolean;
    read:                            boolean;
    user_title:                      string;
    title_is_group:                  boolean;
    bookmarked:                      boolean;
    actions_summary:                 PostActionsSummary[];
    moderator:                       boolean;
    admin:                           boolean;
    staff:                           boolean;
    user_id:                         number;
    hidden:                          boolean;
    trust_level:                     number;
    deleted_at:                      null;
    user_deleted:                    boolean;
    edit_reason:                     null;
    can_view_edit_history:           boolean;
    wiki:                            boolean;
    user_custom_fields:              UserCustomFields;
    reviewable_id:                   number;
    reviewable_score_count:          number;
    reviewable_score_pending_count:  number;
    user_created_at:                 Date;
    user_date_of_birth:              Date;
    reactions:                       Reaction[];
    current_user_reaction?:          CurrentUserReaction;
    reaction_users_count:            number;
    current_user_used_main_reaction: boolean;
    can_accept_answer:               boolean;
    can_unaccept_answer:             boolean;
    accepted_answer:                 boolean;
}

export interface PostActionsSummary {
    id:        number;
    can_act?:  boolean;
	acted?:    boolean;
	count?:    number;
	can_undo?: boolean;
}

export interface UserCustomFields {
    followers: string;
    following: string[];
}

export interface SuggestedTopic {
    id:                     number;
    title:                  string;
    fancy_title:            string;
    slug:                   string;
    posts_count:            number;
    reply_count:            number;
    highest_post_number:    number;
    image_url:              null | string;
    created_at:             Date;
    last_posted_at:         Date;
    bumped:                 boolean;
    bumped_at:              Date;
    archetype:              string;
    unseen:                 boolean;
    last_read_post_number?: number;
    unread?:                number;
    new_posts?:             number;
    unread_posts?:          number;
    pinned:                 boolean;
    unpinned:               null;
    excerpt:                string;
    visible:                boolean;
    closed:                 boolean;
    archived:               boolean;
    notification_level?:    number;
    bookmarked:             boolean | null;
    liked:                  boolean | null;
    tags:                   string[];
    like_count:             number;
    views:                  number;
    category_id:            number;
    featured_link:          null;
    has_accepted_answer:    boolean;
    can_have_answer:        boolean;
    posters:                Poster[];
}

export interface PostCreateResponse {
    action:  string;
    post:    Post;
    success: boolean;
	errors?: string[];
}

export interface Site {
    default_archetype:                        string;
    notification_types:                       { [key: string]: number };
    post_types:                               PostTypes;
    trust_levels:                             TrustLevels;
    groups:                                   Group[];
    filters:                                  string[];
    periods:                                  string[];
    top_menu_items:                           string[];
    anonymous_top_menu_items:                 string[];
    uncategorized_category_id:                number;
    user_field_max_length:                    number;
    post_action_types:                        Type[];
    topic_flag_types:                         Type[];
    can_create_tag:                           boolean;
    can_tag_topics:                           boolean;
    can_tag_pms:                              boolean;
    tags_filter_regexp:                       string;
    top_tags:                                 string[];
    topic_featured_link_allowed_category_ids: number[];
    user_themes:                              UserTheme[];
    user_color_schemes:                       any[];
    default_dark_color_scheme:                DefaultDarkColorScheme;
    censored_regexp:                          string;
    shared_drafts_category_id:                number;
    custom_emoji_translation:                 any;
    watched_words_replace:                    null;
    watched_words_link:                       null;
    categories:                               Category[];
    archetypes:                               Archetype[];
    user_fields:                              any[];
    auth_providers:                           AuthProvider[];
}

export interface Archetype {
    id:      string;
    name:    string;
    options: any[];
}

export interface AuthProvider {
    name:                 string;
    custom_url:           null;
    pretty_name_override: null;
    title_override:       null;
    frame_width:          number | null;
    frame_height:         number | null;
    can_connect:          boolean;
    can_revoke:           boolean;
    icon:                 null;
}

export interface Category {
    id:                                number;
    name:                              string;
    color:                             string;
    text_color:                        string;
    slug:                              string;
    topic_count:                       number;
    post_count:                        number;
    position:                          number;
    description:                       null | string;
    description_text:                  null | string;
    description_excerpt:               null | string;
    topic_url:                         string;
    read_restricted:                   boolean;
    permission:                        number;
    notification_level:                number;
    topic_template:                    null | string;
    has_children:                      boolean;
    sort_order:                        string | null;
    sort_ascending:                    boolean | null;
    show_subcategory_list:             boolean;
    num_featured_topics:               number;
    default_view:                      string | null;
    subcategory_list_style:            any;
    default_top_period:                string;
    default_list_filter:               string;
    minimum_required_tags:             number;
    navigate_to_first_post_after_read: boolean;
    custom_fields:                     CustomFields;
    allowed_tags:                      any[];
    allowed_tag_groups:                string[];
    allow_global_tags:                 boolean;
    min_tags_from_required_group:      number;
    required_tag_group_name:           null | string;
    read_only_banner:                  null | string;
    uploaded_logo:                     UploadedLogo | null;
    uploaded_background:               null;
    can_edit:                          boolean;
    parent_category_id?:               number;
}

export interface CustomFields {
    enable_unassigned_filter: null;
    enable_accepted_answers:  null | string;
}

export interface UploadedLogo {
    id:     number;
    url:    string;
    width:  number;
    height: number;
}

export interface DefaultDarkColorScheme {
    id:              number;
    name:            string;
    version:         number;
    created_at:      Date;
    updated_at:      Date;
    via_wizard:      boolean;
    base_scheme_id:  null;
    theme_id:        null;
    user_selectable: boolean;
}

export interface Group {
    id:             number;
    name:           string;
    flair_url:      null | string;
    flair_bg_color: null | string;
    flair_color:    null | string;
}

export interface Type {
    id:                number | null;
    name_key:          null | string;
    name:              string;
    description:       string;
    short_description: string;
    is_flag:           boolean;
    is_custom_flag:    boolean;
}

export interface PostTypes {
    regular:          number;
    moderator_action: number;
    small_action:     number;
    whisper:          number;
}

export interface TrustLevels {
    newuser: number;
    basic:   number;
    member:  number;
    regular: number;
    leader:  number;
}

export interface UserTheme {
    theme_id:        number;
    name:            string;
    default:         boolean;
    color_scheme_id: number | null;
}
