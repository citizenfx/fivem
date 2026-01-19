# Kibana Dashboard Setup for FiveM Anticheat

This document describes how to set up Kibana dashboards for monitoring the anticheat system.

## Index Pattern Setup

1. Navigate to **Stack Management > Kibana > Index Patterns**
2. Click **Create index pattern**
3. Enter pattern: `anticheat-*`
4. Select `@timestamp` as the time field
5. Click **Create index pattern**

## Dashboard Components

### 1. Detection Overview Dashboard

Create a new dashboard and add the following visualizations:

#### Total Detections (Metric)

```json
{
  "type": "metric",
  "title": "Total Detections (24h)",
  "query": "module:detection",
  "timeRange": "last 24 hours"
}
```

#### Detections by Type (Pie Chart)

- **Type**: Pie chart
- **Filter**: `module:detection`
- **Aggregation**: Terms on `detection_type`
- **Size**: Top 10

#### Detections Over Time (Line Chart)

- **Type**: Line chart
- **Filter**: `module:detection`
- **X-Axis**: Date histogram on `@timestamp` (Auto interval)
- **Y-Axis**: Count

#### Detections by Severity (Bar Chart)

- **Type**: Horizontal bar
- **Filter**: `module:detection`
- **Aggregation**: Terms on `severity`
- Order by count descending

#### Top Flagged Players (Table)

- **Type**: Data table
- **Filter**: `module:detection`
- **Columns**:
  - player_name (Terms)
  - detection_type (Terms)
  - Count

### 2. Sanctions Dashboard

#### Active Bans (Metric)

- **Filter**: `module:sanction AND sanction_type:ban`

#### Sanctions Over Time (Area Chart)

- **Type**: Area chart
- **Filter**: `module:sanction`
- **Split by**: `sanction_type`
- **X-Axis**: Date histogram

#### Recent Sanctions (Saved Search)

- **Type**: Saved search / Discover view
- **Filter**: `module:sanction`
- **Columns**: `@timestamp`, `player_name`, `sanction_type`, `reason`
- **Sort**: `@timestamp` descending

### 3. Real-time Monitor

#### Live Detection Feed

- **Type**: Discover view embedded
- **Filter**: `module:detection OR module:sanction`
- **Auto-refresh**: 5 seconds

## Sample Queries

### High Severity Detections

```
module:detection AND (severity:high OR severity:critical)
```

### Speed Violations

```
module:detection AND detection_type:speed*
```

### Recent Bans

```
module:sanction AND sanction_type:ban
```

### Player Activity

```
player_name:"PlayerName"
```

### API Errors

```
log_source:anticheat-admin AND log.level:error
```

## Alerts (Watcher / Kibana Alerting)

### High Detection Rate Alert

Create an alert when detection count exceeds threshold:

- **Condition**: Detection count > 50 in 5 minutes
- **Action**: Email/Slack notification

### Critical Detection Alert

- **Condition**: Any detection with `severity:critical`
- **Action**: Immediate notification

## Exporting/Importing Dashboards

To export dashboards:

1. Go to **Stack Management > Saved Objects**
2. Select dashboards, visualizations, and index patterns
3. Click **Export**
4. Save the NDJSON file

To import:

1. Go to **Stack Management > Saved Objects**
2. Click **Import**
3. Select the NDJSON file
4. Resolve any conflicts

## Index Lifecycle Management (ILM)

Recommended ILM policy for anticheat logs:

```json
{
  "policy": {
    "phases": {
      "hot": {
        "min_age": "0ms",
        "actions": {
          "rollover": {
            "max_age": "1d",
            "max_size": "5gb"
          }
        }
      },
      "warm": {
        "min_age": "7d",
        "actions": {
          "shrink": {
            "number_of_shards": 1
          }
        }
      },
      "delete": {
        "min_age": "30d",
        "actions": {
          "delete": {}
        }
      }
    }
  }
}
```
