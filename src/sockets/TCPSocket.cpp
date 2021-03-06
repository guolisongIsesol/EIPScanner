//
// Created by Aleksey Timin on 11/16/19.
//

#include <system_error>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "utils/Logger.h"
#include "TCPSocket.h"


namespace eipScanner {
namespace sockets {
	using eipScanner::utils::Logger;
	using eipScanner::utils::LogLevel;

	TCPSocket::TCPSocket(std::string host, int port)
		: TCPSocket(EndPoint(host, port)){
	}

	TCPSocket::TCPSocket(EndPoint endPoint)
			: BaseSocket(std::move(endPoint)) {

		_sockedFd = socket(AF_INET, SOCK_STREAM, 0);
		if (_sockedFd < 0) {
			throw std::system_error(errno, std::generic_category());
		}

		Logger(LogLevel::DEBUG) << "Opened socket fd=" << _sockedFd;

		Logger(LogLevel::DEBUG) << "Connecting to " << _remoteEndPoint.toString();
		auto addr = _remoteEndPoint.getAddr();
		if (connect(_sockedFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			close(_sockedFd);
			throw std::system_error(errno, std::generic_category());
		}
	}

	TCPSocket::~TCPSocket() {
		Logger(LogLevel::DEBUG) << "Close socket fd=" << _sockedFd;
		shutdown(_sockedFd, SHUT_RDWR);
		close(_sockedFd);
	}

	void TCPSocket::Send(const std::vector <uint8_t> &data) const {
		Logger(LogLevel::TRACE) << "Send " << data.size() << " bytes from TCP socket #" << _sockedFd << ".";

		int count = send(_sockedFd, data.data(), data.size(), 0);
		if (count < data.size()) {
			throw std::system_error(errno, std::generic_category());
		}
	}

	std::vector<uint8_t> TCPSocket::Receive(size_t size) const {
		std::vector<uint8_t> recvBuffer(size);

		int count = 0;
		while (size > count) {
			auto len = recv(_sockedFd, recvBuffer.data() + count, size-count, 0);
			count += len;
			if (len < 0) {
				throw std::system_error(errno, std::generic_category());
			}

			if (len == 0) {
				break;
			}
		}

		if (size != count) {
			Logger(LogLevel::WARNING) << "Received from " <<  _remoteEndPoint.toString()
									  << " " << count << " of " << size;
		}

		return recvBuffer;
	}


}
}