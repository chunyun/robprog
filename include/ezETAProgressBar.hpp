/*
Copyright (C) 2011,2012 Remik Ziemlinski. See MIT-LICENSE.

CHANGELOG

v0.0.0 20110502 rsz Created.
V1.0.0 20110522 rsz Extended to show eta with growing bar.
v2.0.0 20110525 rsz Added time elapsed.
v2.0.1 20111006 rsz Added default constructor value.
v2.0.2 20130123 rob Switched over to C++11 timer facilities 
*/

#ifndef EZ_ETAPROGRESSBAR_H
#define EZ_ETAPROGRESSBAR_H

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <cstring>
#include <iomanip>

#ifdef WIN32
#include <windows.h>
#endif

//TODO:isolate timer
namespace ez {
// One-line refreshing progress bar inspired by wget that shows ETA (time
// remaining).
// 90% [========================================>     ] ETA 12d 23h 56s

  namespace details{
//type and shorthand
    using weeks = std::chrono::duration<size_t, std::ratio<604800> >;
    using days = std::chrono::duration<size_t, std::ratio<86400> >;
    using hours = std::chrono::hours;
    using minutes = std::chrono::minutes;
    using seconds = std::chrono::seconds;
    using clock =  std::chrono::system_clock;
    using duration = std::chrono::duration<size_t, clock::period>;
    using time_t = clock::time_point;
    using partialDuration = std::chrono::duration<double, clock::period>;

    std::string durationString(duration t) {
      using std::chrono::duration_cast;

      std::stringstream out(std::stringstream::out);

      if ( t >= days(1) ) {
        auto numDays = duration_cast<days>(t);
        out << numDays.count() << "d ";
        t -= numDays;
      }

      if ( t >= hours(1) ) {
        auto numHours = duration_cast<hours>(t);
        out << numHours.count() << "h ";
        t -= numHours;
      }

      if ( t >= minutes(1) ) {
        auto numMins = duration_cast<minutes>(t);
        out << numMins.count() << "m ";
        t -= numMins;
      }

      if (t >= seconds(1)) {
        auto numSecs = duration_cast<seconds>(t);
        out << numSecs.count() << "s";
      }

      std::string tstring = out.str();
      if (tstring.empty()) { tstring = "0s";  }
      
      return tstring;
    }
  }

class ezETAProgressBar {
private:
//private variable
  uint64_t n;
  uint64_t cur;
  unsigned digits;
  unsigned nticsMax;
  unsigned char width; // How many chars the entire line can be.
  details::time_t startTime, endTime, lastCheck;
  std::ostream& OS;
public:
  ezETAProgressBar(uint64_t _n = 0) : n(_n), cur(0), digits(1),width(80), OS(std::cout) {
    nticsMax = (width - 27 - digits - 1 -1);
  }
  
  void reset(uint64_t _n) { n = _n; cur = 0; }
  
  void done() {  cur = n;  setPct(1.0);  }
  
  void start() {
    using namespace details;
    using namespace std;
    lastCheck = startTime = clock::now();
    OS << " " <<setw(2+3+digits+1) << fixed <<setprecision(digits) << 0 <<"%";
    OS << " [>"<< string(nticsMax,' ')<<"] Estimating...\r"<<flush;
  }

  void operator++() { (*this) += 1; }

  void operator+=(const unsigned int d) {
    using namespace details;
    if (cur >= n)  return;
    
    cur += d;
    
    endTime = clock::now();
    if ( (endTime - lastCheck) >= std::chrono::seconds(1) or (cur == n) ) {
      setPct(static_cast<double>(cur) / n);
      lastCheck = endTime;
    }
  }

  // Set 0.0-1.0, where 1.0 equals 100%.
  void setPct(double Pct) {
    using namespace details;
    using namespace std;
    using std::chrono::duration_cast;
    
    if ( Pct <= 0.0 ) return;

    stringstream os;

    int effLen = 0;

    endTime = clock::now();
    os << " " <<setw(2+3+digits+1) << fixed <<setprecision(digits) << 100*Pct <<"%";

    // Compute how many tics we can display.
    //int nticsMax = (width - 27 - digits - 1 -1);
    unsigned ntics = std::max(0, static_cast<int>(nticsMax * Pct));
    
    os << " [";

#ifdef HAVE_ANSI_TERM
    // Green!
    os << "\e[0;32m" ;
#endif // HAVE_ANSI_TERM

    os << string( ntics , '=');
    os << (Pct == 1.0 ? "=" : ">");
    os << string(nticsMax - ntics, ' ');

#ifdef HAVE_ANSI_TERM
    // Not-Green :(
    os << "\e[0m";
    effLen -= 11;
#endif // HAVE_ANSI_TERM

    os << "] ";

    // Time since we started the progress bar (or reset)
    auto dt = endTime - startTime;

    std::string terminator;
    if (Pct >= 1.0) {
      // job Done,Print overall time and newline.
      os << "Finished in "<<durationString(dt) ;
      terminator = "\n";
    } else {
      // job going on 
      duration esecs = duration_cast<seconds>(dt);
      duration eta = duration_cast<duration>(((esecs * n) / cur) - esecs);
      os << "ETA ";
      if (eta > weeks(1)) {  os << "> 1 week";           } 
      else                {  os << durationString(eta);  }
      terminator = "\r";
    }

    effLen += os.str().size();

    // Pad to remove previously leftover string
    int padding = width - effLen;
    if ( padding > 0 ) {  os<< string( padding , ' ');  }
    OS << os.str() << terminator  << flush;

  }

};
}
#endif // EZ_ETAPROGRESSBAR_H
