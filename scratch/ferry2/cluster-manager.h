#ifndef CLUSTER_MANAGER_H
#define CLUSTER_MANAGER_H

#include <cstdint>
#include <map>
#include <vector>

class ClusterManager
{
public:
    ClusterManager();

    // ========================================
    // ユーザのクラスタ所属管理
    // ========================================

    // 指定したユーザをクラスタへ所属させる
    void AssignUserToCluster(uint32_t userId, int32_t clusterId);

    // 指定したユーザが所属するクラスタIDを取得する
    // 未所属の場合は-1を返す
    int32_t GetClusterId(uint32_t userId) const;

    // 指定したユーザがクラスタへ登録されているか確認する
    bool HasUser(uint32_t userId) const;

    // ========================================
    // クラスタメンバー管理
    // ========================================

    // 指定したクラスタに所属するユーザ一覧を取得する
    std::vector<uint32_t> GetClusterMembers(int32_t clusterId) const;

    // 指定したクラスタが存在するか確認する
    bool HasCluster(int32_t clusterId) const;

    // 現在のクラスタ情報をすべて削除する
    void ClearClusters();

    // ========================================
    // クラスタヘッド管理
    // ========================================

    // 指定したクラスタのクラスタヘッドを設定する
    void SetClusterHead(int32_t clusterId, uint32_t userId);

    // 指定したクラスタにクラスタヘッドが設定されているか確認する
    bool HasClusterHead(int32_t clusterId) const;

    // 指定したクラスタのクラスタヘッドを取得する
    uint32_t GetClusterHead(int32_t clusterId) const;

    // 指定したユーザがクラスタヘッドか確認する
    bool IsClusterHead(uint32_t userId) const;

    // ========================================
    // クラスタ更新
    // ========================================

    // 現在のユーザ情報を基にクラスタを更新する
    void UpdateClusters();

    // ========================================
    // 確認・デバッグ
    // ========================================

    // 現在のクラスタ構成を標準出力へ表示する
    void PrintClusters() const;

private:
    // ユーザID → クラスタID
    std::map<uint32_t, int32_t> m_userClusterMap;

    // クラスタID → 所属ユーザ一覧
    std::map<int32_t, std::vector<uint32_t>> m_clusterMembersMap;

    // クラスタID → クラスタヘッドのユーザID
    std::map<int32_t, uint32_t> m_clusterHeadMap;
};

#endif // CLUSTER_MANAGER_H