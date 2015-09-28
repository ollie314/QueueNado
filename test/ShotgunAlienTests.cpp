
#include "boost/pointer_cast.hpp"
#include <czmq.h>
#include <thread>
#include <boost/thread.hpp>

#include "ShotgunAlienTests.h"
#include "Death.h"
#include "FileIO.h"
void * ShotgunAlienTests::ShotgunThread(void * arg) {
   ShotgunAmmo* ammo = static_cast<ShotgunAmmo*> (arg);
   printf("shooting %d bullet(s) at %s\n", ammo->count, ammo->location.c_str());
   Shotgun* shotgun = new Shotgun;
   shotgun->Aim(ammo->location);
   std::string msg("Fire!");
   //std::cout << "wait for it" << std::endl;
   sleep(ammo->delay);
   //std::cout << "done waiting" << std::endl;
   for (int i = 0; i <= ammo->count && !zctx_interrupted; i++) {
       //std::cout << "bang! " << std::endl;
      shotgun->Fire(msg);
   }
   delete shotgun;
   std::cout << "Shotgun deleted" << std::endl;
   return nullptr;
}
void * ShotgunAlienTests::Buckshothread(void * arg) {
   ShotgunAmmo* ammo = static_cast<ShotgunAmmo*> (arg);
   printf("shooting %d bullet(s) at %s\n", ammo->count, ammo->location.c_str());
   Shotgun shotgun;
   shotgun.Aim(ammo->location);
   std::string msg("Fire!");
   std::vector<std::string> fullShot;
   fullShot.push_back(msg);
   msg.append("again!");
   fullShot.push_back(msg);
   sleep(1);
   for (int i = 0; i <= ammo->count && !zctx_interrupted; i++) {
      shotgun.Fire(fullShot);
   }
   return nullptr;
}
std::string ShotgunAlienTests::GetTcpLocation() {
   int max = 15000;
   int min = 13000;
   int port = (rand()%(max-min))+min;
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(boost::lexical_cast<std::string > (port));
   return tcpLocation;
}

std::string ShotgunAlienTests::GetIpcLocation() {
   int pid = getpid();
   std::string ipcLocation("ipc:///tmp/");
   ipcLocation.append("ShotgunAlienTests");
   ipcLocation.append(boost::lexical_cast<std::string > (pid));
   ipcLocation.append(".ipc");
   return ipcLocation;
}

std::string ShotgunAlienTests::GetInprocLocation() {
   int pid = getpid();
   std::string inprocLocation("inproc://ShotgunAlienTests");
   inprocLocation.append(boost::lexical_cast<std::string > (pid));
   return inprocLocation;
}

TEST_F(ShotgunAlienTests, ShotInTheDark) {
   Shotgun shotgun;
   
   std::string location = ShotgunAlienTests::GetTcpLocation();
   shotgun.Aim(location);
   std::string msg("Fire!");
   shotgun.Fire(msg);
}

TEST_F(ShotgunAlienTests, ShootOneAlienOnce) {
   Alien alien;
   ShotgunAmmo* ammo = new ShotgunAmmo();
   ammo->count = 1;
   ammo->location.assign(ShotgunAlienTests::GetTcpLocation());
   ammo->delay = 10;
   alien.PrepareToBeShot(ammo->location);
   zthread_new(ShotgunAlienTests::ShotgunThread, ammo);
   std::vector<std::string> reply = alien.GetShot();
   int count=0;
   while (reply.empty() && count++ < 100 && !zctx_interrupted) {
      reply = alien.GetShot();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
   ASSERT_EQ(2,reply.size());
   EXPECT_FALSE(reply[0].empty());
   EXPECT_FALSE(reply[1].empty());
   delete ammo;
}


TEST_F(ShotgunAlienTests, ShootOneAlienTenThousandTimesTCP) {
   Alien alien;
   ShotgunAmmo* ammo = new ShotgunAmmo();
   ammo->count = 10000;
   ammo->location.assign(ShotgunAlienTests::GetTcpLocation());
   ammo->delay = 10;
   zthread_new(ShotgunAlienTests::ShotgunThread, ammo);
   alien.PrepareToBeShot(ammo->location);
   int failures = 0;
   for(int i=0; i <= ammo->count && !zctx_interrupted; i++) {
       std::vector<std::string> shots;
       alien.GetShot(1000,shots);

       // See if the queue of shots if full
       if(shots.size() == 0) {
              sleep(ammo->delay);
           std::cout << "Got Zero Length shot" << std::endl;
          i--;
          if (failures++ > 100 ) {
              ASSERT_FALSE(false);
          }
       }
   }
   delete ammo;
}
TEST_F(ShotgunAlienTests, ShootOneAlienOnceMulti) {
   Alien alien;
   ShotgunAmmo* ammo = new ShotgunAmmo();
   ammo->count = 1;
   ammo->location.assign(ShotgunAlienTests::GetTcpLocation());
   zthread_new(ShotgunAlienTests::Buckshothread, ammo);
   alien.PrepareToBeShot(ammo->location);
   
   std::vector<std::string> replies = alien.GetShot();
   ASSERT_EQ(2,replies.size());
   delete ammo;
}
TEST_F(ShotgunAlienTests, ShootOneAlienTenThousandTimesIPC) {
   Alien alien;
   ShotgunAmmo* ammo = new ShotgunAmmo();
   ammo->count = 10000;
   ammo->location.assign(ShotgunAlienTests::GetIpcLocation());
   alien.PrepareToBeShot(ammo->location);
   zthread_new(ShotgunAlienTests::ShotgunThread, ammo);
   alien.GetShot();
   delete ammo;
}


TEST_F(ShotgunAlienTests, AlienThatCantBeShot) {
   Alien alien;
   std::string location("bad_location");
   try {
   alien.PrepareToBeShot(location);
   //should not get here.
   ASSERT_TRUE(false);
   } catch(...) {
      ASSERT_TRUE(true);
   }

}

TEST_F(ShotgunAlienTests, ShotgunBadAim) {
   Shotgun shotgun;
   std::string location("bad_location");
   try {
   shotgun.Aim(location);
   //should not get here.
   ASSERT_TRUE(false);
   } catch(...) {
      ASSERT_TRUE(true);
   }

}

TEST_F(ShotgunAlienTests, ipcFilesCleanedOnNormalExit) {
   std::string target("ipc:///ShotgunAlienTestsExit");
   std::string addressRealPath(target,target.find("ipc://")+6);
   {
      Shotgun stick;
      Alien alien;
      stick.Aim(target);
      alien.PrepareToBeShot(target);
      ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   }
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath)) << "Shotgun did not clean up ipc file: " << addressRealPath;
}

TEST_F(ShotgunAlienTests, ipcFilesCleanedOnFatal) {
   std::string target("ipc:///ShotgunAlienTestsDeath");
   Shotgun stick;
   Alien alien;
   std::string addressRealPath(target,target.find("ipc://")+6);
   Death::SetupExitHandler();
   stick.Aim(target);
   alien.PrepareToBeShot(target);
   ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   CHECK(false);
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath));
}
