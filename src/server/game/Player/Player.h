#ifndef _PLAYER_H
#define _PLAYER_H

#define PROPS_COUNT      16
#define NAME_LENGTH      12

class WorldSession;

struct PlayerInfo
{
	PlayerInfo() :id(0), icon_id(0), sex(0), gold(0), level(0), score(0), all_Chess(0), win_chess(0), 
	win_Rate(0), offline_count(0), start(0), type(0), desk_id(0)
	{
		for (uint8 i = 0; i < PROPS_COUNT; ++i)
		{
			props_count[i] = 0;
		}
		for (uint8 i = 0; i < NAME_LENGTH; ++i)
		{
			account[i] = 0;
			name[i] = 0;
			nick_name[i] = 0;
		}
	}
	uint32 							  id;				           /// �û�Id
	uint32                            icon_id;                     /// �û�ͷ��id
	uint32                            sex;                         /// �Ա� (0--�У�1--Ů)
	uint32                            gold;                        /// �û�ӵ�н����
	uint32                            level;                       /// �û��ȼ�
	uint32 							  score;			           /// �û�����
	uint32                            all_Chess;                   /// �ܾ���
	uint32                            win_chess;                   /// ʤ����
	uint32                            win_Rate;                    /// ʤ��
	uint32                            offline_count;               /// ���ߴ���
	uint32 							  start;			           /// ����Ƿ�ʼ
	uint32							  type;	                       /// ÿ���˵�����(��ʼֵ��1�� 0 - landlord, 1 - farmer)
	uint32					          desk_id;                     /// ���ӱ�ʶ
	uint32                            props_count[PROPS_COUNT];    /// �û�������Ŀ
	uint8                             account[NAME_LENGTH];        /// �û��˺�
	uint8                             name[NAME_LENGTH];           /// �û�����
	uint8                             nick_name[NAME_LENGTH];      /// �û��ǳ�
};

enum PlayerGameStatus
{

};

enum AtQueueFlags
{

};

class Player
{
public:
	explicit Player(WorldSession* session);
	~Player();

	void loadData(PlayerInfo &pInfo);
	uint32 getid(){ return _id; }
	void Update(const uint32 diff);
	bool expiration(){ return  _expiration < 0; }
	void addPlayer(Player *player);
	bool LogOut(){ return false; }
	bool idle(){ return false; }
	bool started(){ return true; }
	void dealCards(uint8 * cards, uint8 * baseCards);
	bool endGame(){ return false; };

private:
	WorldSession* _session;
	uint32 _expiration;
	uint8 _cards[17];
	uint8 _baseCards[3];

	Player *left, *right;
	///// player data
	uint32 							  _id;				            /// �û�Id
	uint32                            _icon_id;                     /// �û�ͷ��id
	uint32                            _sex;                         /// �Ա� (0--�У�1--Ů)
	uint32                            _gold;                        /// �û�ӵ�н����
	uint32                            _level;                       /// �û��ȼ�
	uint32 							  _score;			            /// �û�����
	uint32                            _all_Chess;                   /// �ܾ���
	uint32                            _win_chess;                   /// ʤ����
	uint32                            _win_Rate;                    /// ʤ��
	uint32                            _offline_count;               /// ���ߴ���
	uint32 							  _start;			            /// ����Ƿ�ʼ
	uint32							  _type;	                    /// ÿ���˵�����(��ʼֵ��1�� 0 - landlord, 1 - farmer)
	uint32					          _desk_id;                     /// ���ӱ�ʶ
	uint32                            _props_count[PROPS_COUNT];    /// �û�������Ŀ
	uint8                             _account[NAME_LENGTH];        /// �û��˺�
	uint8                             _name[NAME_LENGTH];           /// �û�����
	uint8                             _nick_name[NAME_LENGTH];      /// �û��ǳ�
};

#endif