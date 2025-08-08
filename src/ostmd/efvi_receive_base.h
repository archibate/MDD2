/*!
 * \file efvi_receive_base.h
 * \date 2023/07/24 10:20
 *
 * \author zhangxuebing
 * Contact: xuebing.zhang@orientfutures.com
 *
 * \brief efvi UDP
 */
#pragma once
#include <thread>
#include <pthread.h>
#include <sched.h>
#include <memory>
#include <chrono>

#include "efvi_receive_depend.h"

class udp_quote_base {
public:
	udp_quote_base() {}
	~udp_quote_base() {
		close();
	}
	bool init(sock_udp_param& param);
	void close();
	virtual void process_packet(const char* data, const unsigned int& len) = 0;
private:
	//初始化efvi相关
	int sl_parse(ef_filter_spec* fs, const char* ip, unsigned short port);
	struct pkt_buf* pkt_buf_from_id(struct resources* res, int pkt_buf_i);
	void pkt_buf_free(struct resources* res, struct pkt_buf* pkt_buf);
	void handle_rx(struct resources* res, int pkt_buf_i, int len);
	void handle_rx_discard(struct resources* res, int pkt_buf_i, int len, int discard_type);
	void refill_rx_ring(struct resources* res);

	//行情接收解析线程
	void bind_thread(std::thread& thread, int core_id);
	void work_thread();

	sock_udp_param				 m_udp_param;						///<组播参数
	bool						 m_receive_quit_flag = false;		///<退出线程的标志
	std::unique_ptr<std::thread> m_recv_thread = nullptr;			///<组播接收线程

	struct resources* m_res = nullptr;								///<行情资源
};