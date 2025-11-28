#ifndef TRACKER_DOMAINS_H
#define TRACKER_DOMAINS_H

// Comprehensive list of tracker and ad domains
// This list includes 200+ known tracking and advertising domains
extern const char *TRACKER_DOMAINS[];
extern const int TRACKER_DOMAINS_COUNT;

// Check if a URL contains a tracker domain
int is_tracker_domain(const char *url);

#endif // TRACKER_DOMAINS_H
