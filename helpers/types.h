#pragma once

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

typedef uint32_t Vertex;
typedef std::size_t Index;

typedef uint32_t StopId;
typedef uint32_t TripId;
typedef uint32_t Time;
typedef uint32_t ConnectionId;

typedef uint16_t CellId;

constexpr Time INF = std::numeric_limits<Time>::max() / 2;

constexpr Vertex noVertex = std::numeric_limits<Vertex>::max();
constexpr Index noIndex = std::numeric_limits<Index>::max();

constexpr CellId noCellId = std::numeric_limits<CellId>::max();
constexpr StopId noStopId = std::numeric_limits<StopId>::max();
constexpr TripId noTripId = std::numeric_limits<TripId>::max();

enum DIRECTION : bool { FWD, BWD };

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double EARTH_RADIUS_METERS = 6371000.0;

struct Coordinate {
  double lat;
  double lon;

  Coordinate(double lat = 0.0, double lon = 0.0) : lat(lat), lon(lon) {}
};

inline double distanceMeters(const Coordinate &a,
                             const Coordinate &b) noexcept {

  const double lat1 = a.lat * DEG_TO_RAD;
  const double lat2 = b.lat * DEG_TO_RAD;
  const double dLat = (b.lat - a.lat) * DEG_TO_RAD;
  const double dLon = (b.lon - a.lon) * DEG_TO_RAD;

  const double sinLat = std::sin(dLat * 0.5);
  const double sinLon = std::sin(dLon * 0.5);

  const double h =
      sinLat * sinLat + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;

  const double c = 2.0 * std::atan2(std::sqrt(h), std::sqrt(1.0 - h));

  return EARTH_RADIUS_METERS * c;
}

inline double fastDistanceMeters(const Coordinate &a,
                                 const Coordinate &b) noexcept {

  const double lat1 = a.lat * DEG_TO_RAD;
  const double lat2 = b.lat * DEG_TO_RAD;

  const double x = (b.lon - a.lon) * DEG_TO_RAD * std::cos((lat1 + lat2) * 0.5);
  const double y = (b.lat - a.lat) * DEG_TO_RAD;

  return EARTH_RADIUS_METERS * std::sqrt(x * x + y * y);
}

struct Stop {
  StopId id;
  std::string name;
  Time change_time;
  Coordinate coordinate;
};

inline double distanceMeters(const Stop &a, const Stop &b) noexcept {
  return distanceMeters(a.coordinate, b.coordinate);
}

struct Connection {
  StopId to;
  Time dep_time;
  Time arr_time;
  TripId trip_id;
};

struct Footpath {
  StopId to;
  Time duration;
};

enum class EdgeType : uint8_t { Connection, Footpath };

struct Predecessor {
  StopId from = noVertex;
  EdgeType type;
  uint32_t index;
};

struct JourneyStep {
  StopId from;
  StopId to;
  EdgeType type;
  uint32_t edgeIdx;
};

static inline std::string formatTime(Time t) {
  int h = t / 3600;
  int m = (t % 3600) / 60;
  int s = t % 60;

  std::ostringstream os;
  os << std::setw(2) << std::setfill('0') << h << ":" << std::setw(2)
     << std::setfill('0') << m << ":" << std::setw(2) << std::setfill('0') << s;
  return os.str();
}

inline uint32_t packTime(Time arr_time, uint8_t num_legs) {
  uint32_t lower = arr_time & 0xFF; // bits 0..7
  uint32_t upper = arr_time >> 8;   // remove lower bits
  return (upper << 13)              // move to bits 13..31
         | ((num_legs & 0x1F) << 8) // bits 8..12
         | lower;
}

inline Time unpackExactArrival(Time packed) {
  uint32_t lower = packed & 0xFF;
  uint32_t upper = packed >> 13;
  return (upper << 8) | lower;
}

inline uint8_t unpackNumLegs(Time packed) { return (packed >> 8) & 0x1F; }
