#ifndef ADES_CODE_ENGINE_API_FRAMEWORK_ZOOARGO_HPP
#define ADES_CODE_ENGINE_API_FRAMEWORK_ZOOARGO_HPP

#include <dlfcn.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <list>
#include <utility>
#include <memory>

namespace mods {
#ifndef T2CWL_IMOD_HPP
#define T2CWL_IMOD_HPP
class IModInterface {
  bool valid{false};

protected:
public:
  IModInterface() = delete;

  IModInterface(const IModInterface &) = delete;

  explicit IModInterface(std::string path) : path_(std::move(path)) {
    handle = dlopen(path_.data(), RTLD_LAZY);
    if (!handle) {
      handle = nullptr;
      setValid(false);
      setLastError(std::string(dlerror()));
    }
    setValid(true);
    resetError();
  }

  virtual ~IModInterface() {
    if (handle != nullptr) {
      dlclose(handle);
      handle = nullptr;
      resetError();
    }
  }

  bool isValid() const { return valid; }
  void setValid(bool val) { IModInterface::valid = val; }

  const std::string &getLastError() const { return lastError; }
  void resetError() { lastError.clear(); }
  void setLastError(std::string strError) {
    IModInterface::lastError = std::move(lastError);
  }

protected:
  std::string lastError{""};
  void *handle{nullptr};

private:
  std::string path_{""};
};
#endif

class WorkflowExecutor : protected mods::IModInterface {

public:
  WorkflowExecutor() = delete;
  explicit WorkflowExecutor(const std::string &path)
      : mods::IModInterface(path) {

    setValid(true);

    start=(int (*)(
        const std::string& configFile,const std::string &cwlFile,const std::string& inputsFile,
        const std::string &wpsServiceID,const std::string &runId,std::string &serviceID))dlsym(handle, "start");
    if (!start) {
      std::cerr << "can't load 'start' function\n";
      setValid(false);
      setLastError("can't load 'start' function");
      return;
    }

    if (isValid()){
      getStatus=(int (*)(const std::string& configFile,
                       const std::string &serviceID, int &percent,std::string &message) )dlsym(handle, "getStatus");
      if (!getStatus) {
        std::cerr << "can't load 'getStatus' function\n";
        setValid(false);
        setLastError("can't load 'getStatus' function");
        return;
      }
    }

    if (isValid()) {
      getResults =
          (int (*)(const std::string &configFile, const std::string &serviceID,
                   std::list<std::pair<std::string, std::string>> &outPutList))
              dlsym(handle, "getResults");

      if (!getResults) {
        std::cerr << "can't load 'getResults' function\n";
        setValid(false);
        setLastError("can't load 'getResults' function");
        return;
      }
    }

    if (isValid()) {
      clear=(void (*)(const std::string &configFile,
                    const std::string &serviceID))  dlsym(handle, "clear");
      if (!clear) {
        std::cerr << "can't load 'clear' function\n";
        setValid(false);
        setLastError("can't load 'clear' function");
        return;
      }
    }

  }

public:
  bool IsValid() { return this->isValid(); }
  std::string GetLastError() {
    return std::move(std::string(this->getLastError()));
  };

  /***
   *
   * @param configFile: the configuration
   * @param cwlFile:  user's cwl
   * @param inputsFile: input json
   * @param wpsServiceID:  wps service ID
   * @param runId: wps run id
   * @param serviceID: fill with libId
   * @return 0 OK
   */
  int (*start)(
      const std::string& configFile,const std::string &cwlFile,const std::string& inputsFile,
      const std::string &wpsServiceID,const std::string &runId,std::string &serviceID){nullptr};

  /***
   *
   * @param configFile: the configuration
   * @param serviceID
   * @param percent
   * @param message
   * @return 0 exit >0 continue
   */
  int (*getStatus)(const std::string& configFile,
                   const std::string &serviceID, int &percent,std::string &message){nullptr};

  int (*getResults)(const std::string& configFile,
                    const std::string &serviceID,
                    std::list<std::pair<std::string, std::string>> &outPutList){ nullptr};

  void (*clear)(const std::string& configFile,
                const std::string &serviceID){ nullptr};

};

} // namespace mods

#endif // ADES_CODE_ENGINE_API_FRAMEWORK_ZOOARGO_HPP